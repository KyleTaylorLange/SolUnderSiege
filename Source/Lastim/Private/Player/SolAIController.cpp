// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolCharacter.h"
#include "Firearm.h"
#include "Projectile.h"
#include "SolGameState.h"
#include "SolPlayerState.h"
#include "SolGameMode.h"
#include "PickupSpawner.h" // Currently only for RoamMap function
#include "SolAIController.h"

/* Using MoveToActor or MoveToLocation seems to sometimes crash the game. Doing it manually seems to work.
// MoveToActor
FAIMoveRequest MoveReq;
//MoveReq.SetNavigationFilter(FilterClass);
MoveReq.SetAllowPartialPath(true); //(bAllowPartialPath);
MoveReq.SetAcceptanceRadius(25.f); //(AcceptableRadius);
MoveReq.SetCanStrafe(true); //(bAllowStrafe);
MoveReq.SetStopOnOverlap(true);  //(bStopOnOverlap);
MoveReq.SetGoalActor(CurrentPickup);
MoveTo(MoveReq);

// MoveToLocation
FAIMoveRequest MoveReq;
//MoveReq.SetNavigationFilter(FilterClass);
MoveReq.SetAllowPartialPath(true); //(bAllowPartialPath);
MoveReq.SetAcceptanceRadius(100.f); //(AcceptableRadius);
MoveReq.SetCanStrafe(false); //(bAllowStrafe);
MoveReq.SetStopOnOverlap(true);  //(bStopOnOverlap);
MoveReq.SetGoalLocation(CurrentEnemy->GetActorLocation());
MoveTo(MoveReq);
*/

ASolAIController::ASolAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bWantsPlayerState = false;
	bWantsNewTask = true;
	bGettingNewTask = false;
	LastUpdateAimErrorTime = 0.0f;
	RotationSpeed = 270.f;
	PeripheralVision = 0.5f;
	CurrentTask = "";
}

void ASolAIController::SetBotProfile(struct FBotProfile InProfile)
{
	BotProfile = InProfile;

	/* Temporary to make alertness affect something. */
	PeripheralVision = 0.5f - (0.25f * BotProfile.Alertness);
	RotationSpeed = 270.f + (90 * BotProfile.Alertness);

	MaxTargetPredictionError = 0.1f + (0.3f * FMath::Abs(BotProfile.Accuracy - 1.0f));
	MaxAimOffsetError = 0.1f + (0.3f * FMath::Abs(BotProfile.Accuracy - 1.0f));
	MaxRecoilCompensationError = 0.1f + (0.3f * FMath::Abs(BotProfile.Accuracy - 1.0f));
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
	//ASolCharacter* MyBot = Cast<ASolCharacter>(GetPawn());
	// Do an AI update if needed.
	if (GetSolPawn() != NULL && AITickTimer <= 0)
	{
		/* Adding randomness to timer also currently helps make semi-auto shots more realistic.
			In time we'll set an actual refire timer to handle that. */
		AITickTimer = FMath::FRandRange(0.10f, 0.25f); //FMath::FRandRange(0.20f, 0.70f);
		
		ScanEnvironment();
		EquipBestWeapon();

		AFirearm* EquippedFirearm = Cast<AFirearm>(GetSolPawn()->GetEquippedWeapon());
		/* Fire at enemy if visible. */
		if (GetEnemy())
		{
			ShootEnemy();
			UpdateAimError(DeltaSeconds);
			AFirearm* MyFirearm = Cast<AFirearm>(GetSolPawn()->GetEquippedWeapon());
			/* Stop reloading if we currently are in a shovel reload. */
			if (MyFirearm && MyFirearm->GetWeaponState() == "Reloading")
			{
				GetSolPawn()->StartFire();
				GetSolPawn()->StopFire();
			}
		}
		/* Otherwise, reload the weapon if we should. */
		if (EquippedFirearm && EquippedFirearm->CanReload() && ((EquippedFirearm->ShouldReload() && CurrentEnemy == NULL) || EquippedFirearm->GetAmmo() <= 0))
		{
			GetSolPawn()->OnReload();
		}
	}
	if (GetSolPawn())
	{
		if (bWantsNewTask && !bGettingNewTask)
		{
			bGettingNewTask = true;
			GetNextTask();
			bGettingNewTask = false;
			bWantsNewTask = false;
		}
	}
	Super::Tick(DeltaSeconds);
}

bool ASolAIController::GetNextTask()
{
	/* Clear enemy if they are dead. 
	TODO: Move this somewhere more logical. */
	if (GetSolPawn() && GetEnemy())
	{
		ASolCharacter* SoldierEnemy = Cast<ASolCharacter>(GetEnemy());
		if (SoldierEnemy && CanSee(SoldierEnemy) && !SoldierEnemy->IsAlive())
		{
			ClearEnemy();
		}
	}
	
	if (GetSolPawn() && GetSolPawn()->IsAlive() && !ShouldPostponePathUpdates())
	{
		/* Only proceed if we have no objective. */
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
					CurrentTask = "Purusing Enemy";
					return true;
				}
			}
			else if (GetPickup() && GetPickup() != nullptr)
			{
				MoveToActor(GetPickup(), 10.f, false, true, true);
				CurrentTask = "Getting Pickup";
				return true;
			}
			/* Fall back to just roaming the map. */
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
	//ASolCharacter* MyBot = Cast<ASolCharacter>(GetPawn());
	FindClosestEnemyWithLOS(nullptr);
	APawn* EnemyToCheck = GetEnemy();
	if (EnemyToCheck && EnemyToCheck != nullptr)
	{
		EnemyLastKnownLocation = EnemyToCheck->GetActorLocation();
	}
	FindClosestPickupWithLOS();
}

void ASolAIController::EquipBestWeapon()
{
	if (GetSolPawn())
	{
		int InvLength = GetSolPawn()->GetInventoryCount();
		AWeapon* BestWeapon = nullptr;
		AWeapon* EquippedWeapon = GetSolPawn()->GetEquippedWeapon();
		int BestWeapIndex = -1;
		float BestRating = 0.f;
		for (int i = 0; i < InvLength; i++)
		{
			// For now, just get the base rating.
			AWeapon* TestWeapon = GetSolPawn()->GetSpecificWeapon(i);
			AFirearm* TestFirearm = Cast<AFirearm>(TestWeapon);
			float TestRating = TestWeapon->GetAIRating();
			// Prefer current weapon, especially if we have a visible enemy.
			if (TestWeapon == EquippedWeapon)
			{
				TestRating *= CurrentEnemy != NULL ? 2.0f : 1.1f;
			}
			// Simple check for ammo: strongly prefer other weapons if we're nearly out of ammo.
			/*
			if (TestFirearm && TestFirearm->GetCurrentClips() <= 0)
			{
				TestRating *= 0.25f;
				if (TestFirearm->GetEnergy() <= 0)
				{
					TestRating = 0.0f;
				}
			}*/
			if (TestRating >= BestRating)
			{
				BestWeapon = TestWeapon;
				BestWeapIndex = i;
				BestRating = TestRating;
			}
		}
		if (GetSolPawn() && BestWeapon && GetSolPawn()->GetPendingWeapon() == nullptr)
		{
			GetSolPawn()->EquipSpecificWeapon(BestWeapIndex);
		}
	}
}

bool ASolAIController::CanSee(AActor* InActor) const
{
	bool bCanSeeObject = false;

	if (InActor == nullptr)
	{
		return false;
	}

	if (InActor != nullptr && GetPawn())
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
			FVector MeshLoc = TestMeshComp->GetCenterOfMass();
			if (TestMeshComp)
			{
				MeshLoc = TestMeshComp->GetCenterOfMass();
				float Desirablility = (TestPickup->GetActorLocation() - MyLoc).SizeSquared();

				//if (CanSee(*It))
				//{
					AWeapon* TestWeapon = Cast<AWeapon>(TestPickup->GetHeldItem());

					if (TestWeapon)
					{
						if (MyBot->GetInventoryCount() >= 3)
						{
							Desirablility = MAX_FLT;
						}
						Desirablility /= TestWeapon->GetAIRating();
					}
					if (Desirablility < BestDesirablility)
					{
						BestDesirablility = Desirablility;
						BestPickup = TestPickup;
						bGotPickup = true;
					}
				//}
			}
		}
		if (BestPickup)
		{
			APickup* OldPickup = GetPickup();
			SetPickup(BestPickup);
			if (CurrentTask == "Getting Pickup" && OldPickup != GetPickup())
			{
				GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished | FPathFollowingResultFlags::ForcedScript);
				bWantsNewTask = true; //GetNextTask();
			}
			// Get the center of the component if possible.
			FVector PickupLoc = CurrentPickup->GetActorLocation();
			USkeletalMeshComponent* PickupMesh = Cast<USkeletalMeshComponent>(CurrentPickup->GetRootComponent());
			if (PickupMesh)
			{
				PickupLoc = PickupMesh->GetCenterOfMass();
			}
			const float Dist = (PickupLoc - MyLoc).SizeSquared();
			// Line of sight check is failing miserably right now, probably because the pickup origin is outside the mesh.
			// We had to do one earlier, so let's assume the bot can truly see the pickup.
			if (Dist <= FMath::Square(100.f)) //(LineOfSightTo(CurrentPickup, MyViewLoc) && Dist <= FMath::Square(125.f))
			{
				MyBot->Crouch();
				bool bLOSToWeapon = CanSee(CurrentPickup);
				bool bUseSuccessful = false;
				if (GetPickup() == MyBot->GetUsableObject())
				{
					bUseSuccessful = MyBot->OnStartUse();
				}
				if (bUseSuccessful)
				{
					ClearPickup();
					//StopMovement();
				}
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
		TestPlayerState = Cast<ASolPlayerState>(InPawn->GetController()->PlayerState);
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
	if (MyBot != NULL)
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
	AFirearm* MyFirearm = Cast<AFirearm>(GetSolPawn()->GetEquippedWeapon());
	if (GetSolPawn() == NULL || MyFirearm == NULL)
	{
		return;
	}
	//if (GetWorldTimerManager().IsTimerActive(TimerHandle_Refire) == true)
	//{
		//return;
	//}

	bool bCanShoot = false;
	if (GetEnemy())
	{
		ASolCharacter* Enemy = Cast<ASolCharacter>(GetEnemy());
		float Distance = 0.0f;
		if (Enemy && (Enemy->IsAlive()) && (MyFirearm->GetAmmo() > 0) && (MyFirearm->CanFire() == true))
		{
			if (HasWeaponLOSToEnemy(Enemy, true))
			{
				bCanShoot = true;
				Distance = FMath::Sqrt((Enemy->GetActorLocation() - GetSolPawn()->GetActorLocation()).SizeSquared());
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
				GetSolPawn()->StartAim();
				////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Starting Aim")));
			}

			GetSolPawn()->StartFire();
			// If semi-automatic, release trigger.
			// For future reference, 0.18 seconds would be a fairly good clicking speed.
			if (!MyFirearm->IsFullAuto())
			{
				GetSolPawn()->StopFire();
				//GetWorldTimerManager().SetTimer(TimerHandle_Refire, this, &ASolAIController::ShootEnemy, FMath::FRandRange(0.25f, 0.5f), false);
			}
			// Ideally the pawn stops moving to fire, but currently they end up firing into wall corners.
			//StopMovement();
		}
		else
		{
			GetSolPawn()->StopFire();
			GetSolPawn()->StopAim();
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
		AimOffsetVector = FVector2D::FVector2D(FMath::FRand() - .5f, FMath::FRand() - .5f).GetSafeNormal();
		AimOffsetError = 5 * MaxAimOffsetError * FMath::FRandRange(-1.0f, 1.0f);
		/* Better accuracy if we're standing still. */
		if (GetSolPawn() && GetSolPawn()->GetVelocity().IsNearlyZero())
		{
			AimOffsetError *= 0.75f;
		}

		/* Error in compensating for recoil. */
		RecoilCompensationError = MaxRecoilCompensationError * FMath::FRandRange(-1.0f, 1.0f);
	}
}

void ASolAIController::AdjustWeaponAim(FVector& FocalPoint)
{
	if (GetSolPawn())
	{
		AFirearm* MyFirearm = Cast<AFirearm>(GetSolPawn()->GetEquippedWeapon());
		if (MyFirearm != NULL && MyFirearm->ProjectileClass.IsValidIndex(MyFirearm->GetCurrentFireMode()))
		{
			
			AProjectile* Proj = MyFirearm->GetModifiedProjectile(MyFirearm->GetCurrentFireMode());
			if (Proj)
			{
				/* Lead shots to hit target. */
				FVector StartLoc = GetSolPawn()->GetWeaponAimLoc();
				FVector EndLoc = FocalPoint;
				float TravelTime = Proj->GetTimeToLocation(StartLoc, EndLoc);
				FVector FinalAimLeadOffset = (TargetVelocity * TravelTime) + (TargetVelocity * TravelTime * TargetPredictionError);
				if (CanSee(FinalAimLeadOffset))
				{
					FocalPoint += FinalAimLeadOffset;
				}

				/* Account for gravity when firing. */
				float GravityZ = Proj->ProjectileMovement->ProjectileGravityScale * (GetSolPawn() ? GetSolPawn()->GetMovementComponent()->GetGravityZ() : GetWorld()->GetGravityZ());
				float ProjSpeed = Proj->ProjectileMovement->InitialSpeed;
				float CollisionRadius = 1.0f; // FIX ME LATER
				bool bHighArc = false;
				FVector TossVel;

				TArray<AActor*> ActorsToIgnore;
				ActorsToIgnore.Add(GetSolPawn());
				if (GetEnemy())
				{
					ActorsToIgnore.Add(GetEnemy());
				}

				if (GravityZ != 0.0f)
				{
					if (UGameplayStatics::SuggestProjectileVelocity(this, TossVel, GetSolPawn()->GetWeaponAimLoc(), FocalPoint, ProjSpeed, bHighArc, CollisionRadius,
						GravityZ, ESuggestProjVelocityTraceOption::TraceFullPath, FCollisionResponseParams::DefaultResponseParam, ActorsToIgnore, true))
					{
						FocalPoint = GetSolPawn()->GetWeaponAimLoc() + TossVel.GetSafeNormal() * 2000.0f;
					}
					else if (UGameplayStatics::SuggestProjectileVelocity(this, TossVel, GetSolPawn()->GetWeaponAimLoc(), FocalPoint, ProjSpeed, true, CollisionRadius,
						GravityZ, ESuggestProjVelocityTraceOption::TraceFullPath, FCollisionResponseParams::DefaultResponseParam, ActorsToIgnore, true))
					{
						FocalPoint = GetSolPawn()->GetWeaponAimLoc() + TossVel.GetSafeNormal() * 2000.0f;
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
	if (GetEnemy() && GetSolPawn())
	{
		ASolCharacter* EnemySoldier = Cast<ASolCharacter>(GetEnemy());
		if (EnemySoldier != nullptr) //|| GetFocalPoint() == EnemyLastKnownLocation)
		{
			// Better aim position; also, uncomment for headshot testing.
			FocalPoint = EnemySoldier->GetTorsoLocation(); //GetHeadLocation();

			AimErrorDistScale = 5000.f / 2500.f + (0.5f * (EnemySoldier->GetActorLocation() - GetSolPawn()->GetActorLocation()).Size());
		}
		AdjustWeaponAim(FocalPoint);
	}
	IdealAimPoint = FocalPoint;
	if (!FocalPoint.IsZero() && GetSolPawn())
	{
		FVector Direction = FocalPoint - GetSolPawn()->GetWeaponAimLoc();
		FRotator DesiredRotation = Direction.Rotation();
		DesiredRotation -= GetSolPawn()->GetWeaponRotationOffset() + (GetSolPawn()->GetWeaponRotationOffset() * RecoilCompensationError);
		DesiredRotation += FRotator(AimOffsetVector.X, AimOffsetVector.Y, 0) * AimOffsetError;
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

class APickup* ASolAIController::GetPickup() const
{
	return CurrentPickup;
}

ASolCharacter* ASolAIController::GetSolPawn()
{
	return Cast<ASolCharacter>(GetPawn());
}