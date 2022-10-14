// Copyright Kyle Taylor Lange

#include "SolAIController.h"
#include "Sol.h"
#include "SolCharacter.h"
#include "Firearm.h"
#include "Projectile.h"
#include "SolGameState.h"
#include "SolPlayerState.h"
#include "SolGameMode.h"
#include "Pickup.h"
#include "PickupSpawner.h" // Currently only for RoamMap function
#include "InventoryComponent.h"

ASolAIController::ASolAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bWantsPlayerState = false;
	bWantsNewTask = true;
	bGettingNewTask = false;
	LastUpdateAimErrorTime = 0.0f;
	RotationSpeed = 270.f;
	PeripheralVision = 0.5f;
	CurrentTask = "";
	bTEMPDebugPathfinding = false;

	bCanPickUpItems = false;
}

void ASolAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ClearEnemy();
	ClearPickup();
	CurrentTask = "";
	bWantsNewTask = true;
}

void ASolAIController::BeginInactiveState()
{
	Super::BeginInactiveState();

	ASolGameState* GameState = Cast<ASolGameState>(GetWorld()->GetGameState());

	// Bot will automatically respawn with bForceRespawn.
	if (!GameState->bForceRespawn)
	{
		ASolPlayerState* MyPlayerState = Cast<ASolPlayerState>(PlayerState);
		const float MinRespawnDelay = (MyPlayerState && GameState && GameState->GameModeClass) ? 
								FMath::Max(MyPlayerState->RespawnTime, GetDefault<AGameMode>(GameState->GameModeClass)->MinRespawnDelay) : 1.0f;

		GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &ASolAIController::Respawn, MinRespawnDelay + FMath::FRand());
	}
}

void ASolAIController::Respawn()
{
	APawn* MyPawn = GetPawn();
	AGameMode* GameMode = Cast<AGameMode>(GetWorld()->GetAuthGameMode());
	if (MyPawn == NULL && GameMode && GameMode->GetMatchState() == MatchState::InProgress)
	{
		GetWorld()->GetAuthGameMode()->RestartPlayer(this);
	}
}

void ASolAIController::Tick(float DeltaSeconds)
{
	// Update AI tick timer
	AITickTimer -= DeltaSeconds;

	// Do an AI update if needed.
	if (GetPawn<ASolCharacter>() && AITickTimer <= 0)
	{
		ASolCharacter* Test = this->GetPawn<ASolCharacter>();
		/* Adding randomness to timer also currently helps make semi-auto shots more realistic.
			In time we'll set an actual refire timer to handle that. */
		AITickTimer = FMath::FRandRange(1.25f, 2.5f);
		
		if (!bTEMPDebugPathfinding)
		{
			ScanEnvironment();
			EquipBestWeapon();
		}

		AFirearm* EquippedFirearm = Cast<AFirearm>(GetPawn<ASolCharacter>()->GetEquippedItem());
		/* Fire at enemy if visible. */
		if (GetEnemy())
		{
			ShootEnemy();
			UpdateAimError(DeltaSeconds);
			AFirearm* MyFirearm = Cast<AFirearm>(GetPawn<ASolCharacter>()->GetEquippedItem());
			/* Stop reloading if we currently are in a shovel reload. */
			if (MyFirearm && MyFirearm->GetWeaponState() == "Reloading")
			{
				GetPawn<ASolCharacter>()->StartFire();
				GetPawn<ASolCharacter>()->StopFire();
			}
		}
		/* Otherwise, reload the weapon if we should. */
		if (EquippedFirearm && EquippedFirearm->CanReload() && ((EquippedFirearm->ShouldReload() && CurrentEnemy == NULL) || EquippedFirearm->GetAmmo() <= 0))
		{
			GetPawn<ASolCharacter>()->OnReload();
		}

		if (bWantsNewTask && !bGettingNewTask)
		{
			bGettingNewTask = true;
			bool bGotNewTask = GetNextTask();
			bGettingNewTask = false;
			bWantsNewTask = !bGotNewTask;
		}
	}
	Super::Tick(DeltaSeconds);
}

bool ASolAIController::GetNextTask()
{
	/* Clear enemy if they are dead. 
	TODO: Move this somewhere more logical. */
	if (GetPawn<ASolCharacter>() && GetEnemy())
	{
		ASolCharacter* SoldierEnemy = Cast<ASolCharacter>(GetEnemy());
		if (SoldierEnemy && CanSee(SoldierEnemy) && !SoldierEnemy->IsAlive())
		{
			ClearEnemy();
		}
	}

	// Temporary "just roam map" code to debug pathfinding.
	if (bTEMPDebugPathfinding && GetPawn<ASolCharacter>() && GetPawn<ASolCharacter>()->IsAlive())
	{
		FVector RoamLoc = RoamMap();
		if (RoamLoc != FVector::ZeroVector)
		{
			//UNavigationPath* NavPath = UNavigationSystem::FindPathToLocationSynchronously(GetWorld(), PathStart, Position, NULL);
			MoveToLocation(RoamLoc, 10.f, false, true, false, false);
			CurrentTask = "Roam";

			bool bHasValidPath = GetPathFollowingComponent()->HasValidPath();
			bool bHasPartialPath = GetPathFollowingComponent()->HasPartialPath();
			if (!bHasValidPath)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s has an invalid path to %s."), *this->GetName(), *RoamLoc.ToString()));
			}
			else if (bHasPartialPath)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s has a partial path to %s."), *this->GetName(), *RoamLoc.ToString()));
				//StopMovement();

			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s has a valid path to %s."), *this->GetName(), *RoamLoc.ToString()));
			}
			return bHasValidPath && !bHasPartialPath;
		}
	}
	
	if (GetPawn<ASolCharacter>() && GetPawn<ASolCharacter>()->IsAlive() && !ShouldPostponePathUpdates())
	{
		// Only proceed if we have no objective.
		if (!CheckObjective())
		{
			if (GetEnemy() && GetEnemy() != nullptr)
			{
				if (!bHasLostEnemy)
				{
					MoveToActor(GetEnemy(), 2500.f, false, true, true);
					SetFocus(GetEnemy());
					CurrentTask = "Attacking Enemy";
					return true;
				}
				else if (EnemyLastKnownLocation != FVector::ZeroVector)
				{
					MoveToLocation(EnemyLastKnownLocation, 10.f, false, true, true);
					SetFocalPoint(EnemyLastKnownLocation);
					CurrentTask = "Pursuing Enemy";
					return true;
				}
			}
			else if (GetPickup() && GetPickup() != nullptr)
			{
				MoveToActor(GetPickup(), 10.f, false, false, true);
				SetFocus(GetPickup());
				CurrentTask = "Getting Pickup";
				return true;
			}
			// Fall back to just roaming the map.
			else if (GetMoveStatus() == EPathFollowingStatus::Idle || !IsFollowingAPath())
			{
				FVector RoamLoc = RoamMap();
				if (RoamLoc != FVector::ZeroVector)
				{
					MoveToLocation(RoamLoc, 10.f, false, true, false, false);
					CurrentTask = "Roam";
					return true;
				}
			}
		}
	}
	CurrentTask = "";
	return false;
}

bool ASolAIController::CheckObjective()
{
	/* By default, there are no game-specific tasks. This can be overridden for different gametypes. */
	return false;
}

void ASolAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	// TEMP: Clear enemy if we went to their last known location and have no visible targets.
	if (bHasLostEnemy && !FindClosestEnemyWithLOS(nullptr))
	{
		ClearEnemy();
	}
	/* If we successfully moved somewhere, we probably need a new task. */
	bWantsNewTask = true; //GetNextTask();
}

void ASolAIController::ScanEnvironment()
{
	FindClosestEnemyWithLOS(nullptr);
	if (GetEnemy())
	{
		EnemyLastKnownLocation = GetEnemy()->GetActorLocation();
	}
	if (bCanPickUpItems)
	{
		FindClosestPickupWithLOS();
	}
}

void ASolAIController::EquipBestWeapon()
{
	if (GetPawn<ASolCharacter>())
	{
		TArray<AInventoryItem*> Inventory = GetPawn<ASolCharacter>()->GetInventoryComponent()->GetInventory();
		int InvLength = Inventory.Num();
		AWeapon* BestWeapon = nullptr;
		AWeapon* EquippedWeapon = Cast<AWeapon>(GetPawn<ASolCharacter>()->GetEquippedItem());
		int BestWeapIndex = -1;
		float BestRating = 0.f;
		for (auto TestItem : Inventory)
		{
			// For now, just get the base rating. Do not equip anything with zero rating.
			float TestRating = 0.f;
			AWeapon* TestWeapon = Cast<AWeapon>(TestItem);
			if (TestWeapon)
			{
				TestRating = TestWeapon->GetAIRating();
				// Prefer current weapon, especially if we have a visible enemy.
				if (TestWeapon == EquippedWeapon)
				{
					TestRating *= CurrentEnemy != NULL ? 2.0f : 1.1f;
				}

				AFirearm* TestFirearm = Cast<AFirearm>(TestWeapon);
				if (TestFirearm)
				{
					if (TestFirearm && TestFirearm->GetAmmoForFireMode(TestFirearm->GetCurrentFireMode()) <= 0)
					{
						TestRating = 0.0f;
					}
				}
			}
			if (TestRating >= BestRating && TestRating > 0.f)
			{
				BestWeapon = TestWeapon;
				BestRating = TestRating;
			}
		}
		if (GetPawn<ASolCharacter>() && BestWeapon && GetPawn<ASolCharacter>()->GetPendingItem() == nullptr)
		{
			GetPawn<ASolCharacter>()->EquipItem(BestWeapon);
		}
	}
}

bool ASolAIController::CanSee(AActor* InActor) const
{
	bool bCanSeeObject = false;

	if (!InActor)
	{
		return false;
	}

	if (InActor && GetPawn())
	{
		FVector MyViewVector = GetPawn()->GetControlRotation().Vector();
		FVector IdealViewVector = (InActor->GetActorLocation() - GetPawn()->GetActorLocation());
		//IdealViewVector.Normalize();
		float DotProduct = FVector::DotProduct(MyViewVector.GetSafeNormal(), IdealViewVector);
		if (DotProduct > PeripheralVision)
		{
			static FName LosTag = FName(TEXT("AIWeaponLosTrace"));
			FCollisionQueryParams TraceParams(LosTag, true, GetPawn());
			// Depreciated in 4.22?
			//TraceParams.bTraceAsyncScene = true;

			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(GetPawn());
			ActorsToIgnore.Add(InActor);
			TraceParams.AddIgnoredActors(ActorsToIgnore);

			FVector StartLocation = MyViewVector;

			FHitResult Hit(ForceInit);
			const FVector EndLocation = IdealViewVector;
			//bCanSeeObject = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_PROJECTILE, TraceParams);



			FVector MyViewLocation = GetPawn()->GetActorLocation() + FVector(0.f, 0.f, GetPawn()->BaseEyeHeight);
			bCanSeeObject = LineOfSightTo(InActor, MyViewLocation);
		}
	}

	return bCanSeeObject;
}

bool ASolAIController::CanSee(FVector InVector) const
{
	bool bCanSeeLocation = false;

	if (GetPawn())
	{
		FVector MyViewVector = GetPawn()->GetControlRotation().Vector();
		FVector IdealViewVector = (InVector - GetPawn()->GetActorLocation());
		float DotProduct = FVector::DotProduct(MyViewVector.GetSafeNormal(), IdealViewVector);
		if (DotProduct > 0.5f) //Give them 120 degree vision for now since they're otherwise morons.
		{
			static FName LosTag = FName(TEXT("AIWeaponLosTrace"));
			FCollisionQueryParams TraceParams(LosTag, true, GetPawn());
			// Depreciated in 4.22?
			//TraceParams.bTraceAsyncScene = true;

			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(GetPawn());
			TraceParams.AddIgnoredActors(ActorsToIgnore);

			FVector StartLocation = MyViewVector;

			FHitResult Hit(ForceInit);
			const FVector EndLocation = IdealViewVector;
			bCanSeeLocation = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_PROJECTILE, TraceParams);

			//bCanSeeLocation = true; //LineOfSightTo(InVector, MyViewLocation);
		}
	}

	return bCanSeeLocation;
}

bool ASolAIController::FindClosestPickupWithLOS()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("Finding Pickup w/ LOS")));
	bool bGotPickup = false;
	ASolCharacter* MyBot = Cast<ASolCharacter>(GetPawn());
	if (MyBot != NULL)
	{
		const FVector MyLoc = MyBot->GetActorLocation();
		float BestDesirablility = MAX_FLT;
		APickup* BestPickup = NULL;

		for (TActorIterator<APickup> It(GetWorld()); It; ++It)
		{
			APickup* TestPickup = *It;
			UMeshComponent* TestMeshComp = TestPickup->PickupMesh;
			FVector MeshLoc = TestPickup->GetActorLocation(); //TestMeshComp->GetCenterOfMass();
			if (TestMeshComp)
			{
				MeshLoc = TestMeshComp->GetCenterOfMass();
				float Desirablility = (TestPickup->GetActorLocation() - MyLoc).SizeSquared();

				if (CanSee(*It) || LineOfSightTo(*It))
				{
					/**
					AWeapon* TestWeapon = Cast<AWeapon>(TestPickup->GetHeldItem());

					if (TestWeapon)
					{
						if (MyBot->GetInventoryCount() >= 3)
						{
							Desirablility = MAX_FLT;
						}
						Desirablility /= TestWeapon->GetAIRating();
					}*/
					if (Desirablility < BestDesirablility)
					{
						BestDesirablility = Desirablility;
						BestPickup = TestPickup;
						bGotPickup = true;
					}
				}
			}
		}
		if (BestPickup)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("Best Pickup Found!")));
			APickup* OldPickup = GetPickup();
			SetPickup(BestPickup);
			if (CurrentTask == "Getting Pickup" && OldPickup != GetPickup())
			{
				GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished | FPathFollowingResultFlags::ForcedScript);
				bWantsNewTask = true; //GetNextTask();
			}
			// Get the center of the component if possible.
			FVector PickupLoc = CurrentPickup->GetActorLocation();
			UMeshComponent* PickupMesh = Cast<UMeshComponent>(CurrentPickup->PickupMesh);
			if (PickupMesh)
			{
				//PickupLoc = PickupMesh->GetCenterOfMass();
			}
			const float Dist = (PickupLoc - MyLoc).SizeSquared();
			// Line of sight check is failing miserably right now, probably because the pickup origin is outside the mesh.
			// We had to do one earlier, so let's assume the bot can truly see the pickup.
			if (Dist <= FMath::Square(100.f)) //(LineOfSightTo(CurrentPickup, MyViewLoc) && Dist <= FMath::Square(125.f))
			{
				MyBot->Crouch();
				bool bUseSuccessful = false;
				TSubclassOf<UInteractionEvent> Interaction;
				UInteractableComponent* Interactable = MyBot->FindInteractable(Interaction);
				if (Interactable && GetPickup() == Interactable->GetOwner())
				{
					StopMovement();
					MyBot->OnStartUse();
				}
				ClearPickup();
				MyBot->UnCrouch();
			}
		}
		else if (CurrentPickup)
		{
			ClearPickup();
			//StopMovement();
		}
	}
	return bGotPickup;
}

bool ASolAIController::FindClosestEnemyWithLOS(ASolCharacter* ExcludeEnemy)
{
	bool bGotEnemy = false;
	APawn* MyBot = GetPawn();
	if (MyBot != NULL)
	{
		const FVector MyLoc = MyBot->GetActorLocation();
		float BestDistSq = MAX_FLT;
		ASolCharacter* BestPawn = NULL;

		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			ASolCharacter* TestPawn = Cast<ASolCharacter>(*It);
			if (TestPawn && CanSee(TestPawn) && TestPawn != MyBot && TestPawn != ExcludeEnemy && TestPawn->IsAlive() && IsEnemy(TestPawn))
			{
				const float DistSq = (TestPawn->GetActorLocation() - MyLoc).SizeSquared();
				if (DistSq < BestDistSq)
				{
					BestDistSq = DistSq;
					BestPawn = TestPawn;
				}
			}
		}
		// Follow current target, otherwise go to last enemy's last location.
		if (BestPawn)
		{
			APawn* OldEnemy = GetEnemy();
			SetEnemy(BestPawn);
			if (CurrentTask == "Attacking Enemy" && OldEnemy != GetEnemy() && GetEnemy() != nullptr)
			{
				GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished | FPathFollowingResultFlags::ForcedScript);
				bWantsNewTask = true; //GetNextTask();
			}
			EnemyLastKnownLocation = CurrentEnemy->GetActorLocation();
			TargetVelocity = CurrentEnemy->GetVelocity();
			bHasLostEnemy = false;
			bGotEnemy = true;
		}
		// Go to the enemy's last known location.
		// NOTE: Due to this function being called every .25-.75 seconds, they rarely follow into other rooms like intended.
		else if (CurrentEnemy != NULL && !bHasLostEnemy)
		{
			bHasLostEnemy = true;
		}
	}
	return bGotEnemy;
}

bool ASolAIController::IsEnemy(APawn* InPawn)
{
	if (InPawn == nullptr || InPawn == GetPawn())
	{
		return false;
	}

	ASolPlayerState* MyPlayerState = Cast<ASolPlayerState>(PlayerState);
	ASolPlayerState* TestPlayerState = nullptr;
	if (InPawn->GetController() && InPawn->GetController()->PlayerState)
	{
		TestPlayerState = InPawn->GetController()->GetPlayerState<ASolPlayerState>();
	}

	bool bIsEnemy = true;
	if (GetWorld()->GetGameState() && GetWorld()->GetGameState()->GameModeClass)
	{
		const ASolGameMode* DefGame = GetWorld()->GetGameState()->GameModeClass->GetDefaultObject<ASolGameMode>();
		if (DefGame && MyPlayerState && TestPlayerState)
		{
			bIsEnemy = DefGame->PlayersAreEnemies(TestPlayerState, MyPlayerState);
		}
	}

	return bIsEnemy;
}

bool ASolAIController::HasLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const
{
	static FName LosTag = FName(TEXT("AIWeaponLosTrace"));

	ASolCharacter* MyBot = Cast<ASolCharacter>(GetPawn()); // ASolCharacter was AShooterBot

	bool bHasLOS = false;
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(LosTag, true, GetPawn());
	// Depreciated in 4.22?
	//TraceParams.bTraceAsyncScene = true;

	TraceParams.bReturnPhysicalMaterial = true;
	FVector StartLocation = MyBot->GetActorLocation();
	StartLocation.Z += GetPawn()->BaseEyeHeight; //look from eyes

	FHitResult Hit(ForceInit);
	const FVector EndLocation = InEnemyActor->GetActorLocation();
	GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_PROJECTILE, TraceParams); //ECC_GameTraceChannel1 == COLLISION_WEAPON
	if (Hit.bBlockingHit == true)
	{
		// Theres a blocking hit - check if its our enemy actor
		AActor* HitActor = Hit.GetActor();
		if (Hit.GetActor() != NULL)
		{
			if (HitActor == InEnemyActor)
			{
				bHasLOS = true;
			}
			else if (bAnyEnemy == true)
			{
				// It's not our actor, maybe it's still an enemy ?
				ACharacter* HitChar = Cast<ACharacter>(HitActor);
				if (HitChar != NULL)
				{
					ASolPlayerState* HitPlayerState = Cast<ASolPlayerState>(HitChar->GetPlayerState());
					ASolPlayerState* MyPlayerState = Cast<ASolPlayerState>(PlayerState);
					if ((HitPlayerState != NULL) && (MyPlayerState != NULL))
					{
						if (HitPlayerState->GetTeam() != MyPlayerState->GetTeam())
						{
							bHasLOS = true;
						}
					}
				}
			}
		}
	}
	return bHasLOS;
}

bool ASolAIController::HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const
{
	static FName LosTag = FName(TEXT("AIWeaponLosTrace"));

	ASolCharacter* MyBot = Cast<ASolCharacter>(GetPawn()); // ASolCharacter was AShooterBot

	bool bHasLOS = false;
	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(LosTag, true, GetPawn());
	// Depreciated in 4.22?
	//TraceParams.bTraceAsyncScene = true;

	TraceParams.bReturnPhysicalMaterial = true;
	FVector StartLocation = MyBot->GetWeaponAimLoc(); //MyBot->GetActorLocation();
	//StartLocation.Z += GetPawn()->BaseEyeHeight; //look from eyes

	FHitResult Hit(ForceInit);
	//const FVector EndLocation = InEnemyActor->GetActorLocation();
	//TODO: Get range from weapon (bots shouldn't try to snipe with Blunderbuss).
	const float Range = 10000000000000000000.f;

	FVector AimLocation = MyBot->GetControlRotation().Vector();
	FVector EnemyLocation = IdealAimRotation.Vector();   //(IdealAimPoint - MyBot->GetWeaponAimLoc()).GetSafeNormal();
	float DotProduct = FVector::DotProduct(AimLocation, EnemyLocation);

	float TempAccuracyTest = 0.85f; //0.75f + (0.15f * BotProfile.Accuracy);
	if (CanSee(InEnemyActor) && DotProduct > TempAccuracyTest)
	{
		// For now, just go ahead and fire if we're close enough.
		// We'll have to merge the above code with the below unused code for best behaviour.
		return true;
	}



	//const FVector EndLocation = StartLocation + ((MyBot->GetWeaponAimRot() - AimError).Vector() * Range); //(MyBot->GetControlRotation().Vector() * Range);
	const FVector EndLocation = StartLocation + (MyBot->GetWeaponAimRot().Vector() * Range); //(MyBot->GetControlRotation().Vector() * Range);
	GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, COLLISION_PROJECTILE, TraceParams);
	if (Hit.bBlockingHit == true)
	{
		// There's a blocking hit - check if it's our enemy actor
		AActor* HitActor = Hit.GetActor();
		if (Hit.GetActor() != NULL)
		{
			if (HitActor == InEnemyActor)
			{
				bHasLOS = true;
			}
			// It's not our actor, maybe it's still an enemy?
			else if (bAnyEnemy == true)
			{
				ACharacter* HitChar = Cast<ACharacter>(HitActor);
				if (HitChar != NULL)
				{
					ASolPlayerState* HitPlayerState = Cast<ASolPlayerState>(HitChar->GetPlayerState());
					ASolPlayerState* MyPlayerState = Cast<ASolPlayerState>(PlayerState);
					if ((HitPlayerState != NULL) && (MyPlayerState != NULL))
					{
						if (HitPlayerState->GetTeamNum() != MyPlayerState->GetTeamNum())
						{
							bHasLOS = true;
						}
					}
				}
			}
		}
	}
	return bHasLOS;
}

FVector ASolAIController::RoamMap()
{
	bool bGotLocation = false;
	FVector Destination = FVector::ZeroVector;
	APawn* MyBot = GetPawn();
	if (MyBot)
	{
		const FVector MyLoc = MyBot->GetActorLocation();
		float BestDistSq = 0;

		// Find a random pickup spawner to go to.
		for (TActorIterator<APickupSpawner> It(GetWorld()); It; ++It)
		{
			APickupSpawner* TestLoc = *It;
			// Add some randomness so they don't just go between the same two points.
			const float DistSq = (TestLoc->GetActorLocation() - MyLoc).SizeSquared() * FMath::FRand();
			if (DistSq > BestDistSq)
			{
				BestDistSq = DistSq;
				Destination = TestLoc->GetActorLocation();
				bGotLocation = true;
			}
		}
		// If there are no weapon spawns, then go to a player start.
		if (!bGotLocation)
		{
			for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
			{
				APlayerStart* TestLoc = *It;
				// Add some randomness so they don't just go between the same two points.
				const float DistSq = (TestLoc->GetActorLocation() - MyLoc).SizeSquared() * FMath::FRand();
				if (DistSq > BestDistSq)
				{
					BestDistSq = DistSq;
					Destination = TestLoc->GetActorLocation();
					bGotLocation = true;
				}
			}
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Roaming Map")));

	return Destination;
}

void ASolAIController::SetEnemy(class APawn* InPawn)
{
	CurrentEnemy = InPawn;
}

void ASolAIController::ClearEnemy()
{
	bool bCheckTask = false;
	if (GetFocusActor() == CurrentEnemy)
	{
		bCheckTask = true;
		ClearFocus(EAIFocusPriority::Gameplay);
	}
	CurrentEnemy = nullptr;
	if (bCheckTask)
	{
		bWantsNewTask = true; //GetNextTask();
	}
}

void ASolAIController::SetPickup(class APickup* InPickup)
{
	CurrentPickup = InPickup;
}

void ASolAIController::ClearPickup()
{
	bool bCheckTask = false;
	if (GetFocusActor() == CurrentPickup)
	{
		bCheckTask = true;
		ClearFocus(EAIFocusPriority::Gameplay);
	}
	CurrentPickup = nullptr;
	if (bCheckTask)
	{
		bWantsNewTask = true; //GetNextTask();
	}
}

void ASolAIController::ShootEnemy()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Refire))
	{
		return;
	}
	if (!GetPawn<ASolCharacter>())
	{
		return;
	}
	AFirearm* MyFirearm = Cast<AFirearm>(GetPawn<ASolCharacter>()->GetEquippedItem());
	if (!MyFirearm)
	{
		return;
	}

	bool bCanShoot = false;
	if (GetEnemy())
	{
		ASolCharacter* Enemy = GetEnemy< ASolCharacter>();
		float Distance = 0.0f;
		if (Enemy && (Enemy->IsAlive()) && (MyFirearm->GetAmmo() > 0) && (MyFirearm->CanFire() == true))
		{
			if (HasWeaponLOSToEnemy(Enemy, true))
			{
				bCanShoot = true;
				Distance = FMath::Sqrt((Enemy->GetActorLocation() - GetPawn<ASolCharacter>()->GetActorLocation()).SizeSquared());
			}
		}
		// Clear the enemy if they're dead.
		// TODO: Check that the bot has seen the enemy's body.
		else if (!Enemy->IsAlive())
		{
			ClearEnemy();
		}
		if (bCanShoot)
		{
			if (Distance > 2500.f)
			{
				GetPawn<ASolCharacter>()->StartAim();
				////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Starting Aim")));
			}

			GetPawn<ASolCharacter>()->StartFire();
			// If semi-automatic, release trigger.
			// For future reference, 0.18 seconds would be a fairly good clicking speed.
			if (!MyFirearm->IsFullAuto())
			{
				GetPawn<ASolCharacter>()->StopFire();
				GetWorldTimerManager().SetTimer(TimerHandle_Refire, this, &ASolAIController::ShootEnemy, FMath::FRandRange(0.25f, 0.5f), false);
			}
			// Ideally the pawn stops moving to fire, but currently they end up firing into wall corners.
			//StopMovement();
		}
		else
		{
			GetPawn<ASolCharacter>()->StopFire();
			GetPawn<ASolCharacter>()->StopAim();
		}
	}
}

void ASolAIController::UpdateAimError(float DeltaSeconds)
{
	if (LastUpdateAimErrorTime + 0.25f <= GetWorld()->GetTimeSeconds())
	{
		LastUpdateAimErrorTime = GetWorld()->GetTimeSeconds();

		/* Error for leading targets. */
		TargetPredictionError = MaxTargetPredictionError * FMath::FRandRange(-1.0f, 1.0f);

		/* Standard error. */
		AimOffsetVector = FVector2D(FMath::FRand() - .5f, FMath::FRand() - .5f).GetSafeNormal();
		AimOffsetError = 5 * MaxAimOffsetError * FMath::FRandRange(-1.0f, 1.0f);
		/* Better accuracy if we're standing still. */
		if (GetPawn<ASolCharacter>() && GetPawn<ASolCharacter>()->GetVelocity().IsNearlyZero())
		{
			AimOffsetError *= 0.75f;
		}

		/* Error in compensating for recoil. */
		RecoilCompensationError = MaxRecoilCompensationError * FMath::FRandRange(-1.0f, 1.0f);
	}
}

void ASolAIController::AdjustWeaponAim(FVector& FocalPoint)
{
	if (GetPawn<ASolCharacter>())
	{
		AFirearm* MyFirearm = Cast<AFirearm>(GetPawn<ASolCharacter>()->GetEquippedItem());
		if (MyFirearm != NULL && MyFirearm->ProjectileClass.IsValidIndex(MyFirearm->GetCurrentFireMode()))
		{
			
			AProjectile* Proj = MyFirearm->GetModifiedProjectile(MyFirearm->GetCurrentFireMode());
			if (Proj)
			{
				/* Lead shots to hit target. */
				FVector StartLoc = GetPawn<ASolCharacter>()->GetWeaponAimLoc();
				FVector EndLoc = FocalPoint;
				float TravelTime = Proj->GetTimeToLocation(StartLoc, EndLoc);
				FVector FinalAimLeadOffset = (TargetVelocity * TravelTime) + (TargetVelocity * TravelTime * TargetPredictionError);
				if (CanSee(FinalAimLeadOffset))
				{
					FocalPoint += FinalAimLeadOffset;
				}

				/* Account for gravity when firing. */
				float GravityZ = Proj->ProjectileMovement->ProjectileGravityScale * (GetPawn<ASolCharacter>() ? GetPawn<ASolCharacter>()->GetMovementComponent()->GetGravityZ() : GetWorld()->GetGravityZ());
				float ProjSpeed = Proj->ProjectileMovement->InitialSpeed;
				float CollisionRadius = 1.0f; // FIX ME LATER
				bool bHighArc = false;
				FVector TossVel;

				TArray<AActor*> ActorsToIgnore;
				ActorsToIgnore.Add(GetPawn<ASolCharacter>());
				if (GetEnemy())
				{
					ActorsToIgnore.Add(GetEnemy());
				}

				if (GravityZ != 0.0f)
				{
					if (UGameplayStatics::SuggestProjectileVelocity(this, TossVel, GetPawn<ASolCharacter>()->GetWeaponAimLoc(), FocalPoint, ProjSpeed, bHighArc, CollisionRadius,
						GravityZ, ESuggestProjVelocityTraceOption::TraceFullPath, FCollisionResponseParams::DefaultResponseParam, ActorsToIgnore, true))
					{
						FocalPoint = GetPawn<ASolCharacter>()->GetWeaponAimLoc() + TossVel.GetSafeNormal() * 2000.0f;
					}
					else if (UGameplayStatics::SuggestProjectileVelocity(this, TossVel, GetPawn<ASolCharacter>()->GetWeaponAimLoc(), FocalPoint, ProjSpeed, true, CollisionRadius,
						GravityZ, ESuggestProjVelocityTraceOption::TraceFullPath, FCollisionResponseParams::DefaultResponseParam, ActorsToIgnore, true))
					{
						FocalPoint = GetPawn<ASolCharacter>()->GetWeaponAimLoc() + TossVel.GetSafeNormal() * 2000.0f;
					}
				}
			}
		}
	}
}

void ASolAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	// Look toward focus
	FVector FocalPoint = GetFocalPoint();
	float AimErrorDistScale = 0.f;
	if (GetEnemy() && GetPawn<ASolCharacter>())
	{
		ASolCharacter* EnemySoldier = Cast<ASolCharacter>(GetEnemy());
		if (EnemySoldier != nullptr) //|| GetFocalPoint() == EnemyLastKnownLocation)
		{
			// Better aim position; also, uncomment for headshot testing.
			FocalPoint = EnemySoldier->GetTorsoLocation(); //GetHeadLocation();

			AimErrorDistScale = 5000.f / 2500.f + (0.5f * (EnemySoldier->GetActorLocation() - GetPawn<ASolCharacter>()->GetActorLocation()).Size());
		}
		AdjustWeaponAim(FocalPoint);
	}
	IdealAimPoint = FocalPoint;
	if (!FocalPoint.IsZero() && GetPawn<ASolCharacter>())
	{
		FVector Direction = FocalPoint - GetPawn<ASolCharacter>()->GetWeaponAimLoc();
		FRotator DesiredRotation = Direction.Rotation();
		IdealAimRotation = DesiredRotation;

		float FinalRotSpeed = RotationSpeed;
		float DotProduct = FVector::DotProduct(ControlRotation.Vector(), DesiredRotation.Vector());
		/* Rotate slower if we're trying to aim. */
		if (DotProduct >= 0.0f)
		{
			FinalRotSpeed *= 1.f - (DotProduct - 0.2f);
		}

		FRotator NewControlRotation(FMath::FixedTurn(ControlRotation.Pitch, DesiredRotation.Pitch, FinalRotSpeed * DeltaTime),
									FMath::FixedTurn(ControlRotation.Yaw, DesiredRotation.Yaw, FinalRotSpeed * DeltaTime), 0);

		NewControlRotation.Yaw = FRotator::ClampAxis(NewControlRotation.Yaw);

		SetControlRotation(NewControlRotation);

		APawn* const P = GetPawn();
		if (P && bUpdatePawn)
		{
			P->FaceRotation(NewControlRotation, DeltaTime);
		}

	}
}

class APawn* ASolAIController::GetEnemy() const
{
	return CurrentEnemy;
}

template<class T>
T* ASolAIController::GetEnemy() const
{
	return Cast<T>(CurrentEnemy);
}

class APickup* ASolAIController::GetPickup() const
{
	return CurrentPickup;
}
