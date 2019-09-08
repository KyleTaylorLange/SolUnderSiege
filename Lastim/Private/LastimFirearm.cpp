// Fill out your copyright notice in the Description page of Project Settings.

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimProjectile.h"
#include "Bullet.h" // To make this the default projectile.
#include "LastimAmmo.h"
#include "LastimCharacter.h"
#include "LastimFirearm.h"

//////////////////////////////////////////////////////////////////////////
// ALastimFirearm

ALastimFirearm::ALastimFirearm(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bWantsToFire = false;
	bPendingReload = false;
	static ConstructorHelpers::FObjectFinder<USoundWave> AudioObj(TEXT("/Game/Sounds/ShooterGame/Weapon_AssaultRifle_Empty01.Weapon_AssaultRifle_Empty01"));
	OutOfAmmoSound = AudioObj.Object;

	SpreadRadius = 5.f; // 5 cm
	SpreadRange = 0.f; // Perfect accuracy by default.
	LastFireTime = 0.0f;
	RechargePct = 0.0f;
	BurstCounter = 0;
	CurrentFireMode = 0;
	MaxFireModes = 1;

	ProjectileCount.SetNum(1);
	ProjectileCount[0] = 1;

	ProjectileClass.SetNum(1);
	ProjectileClass[0] = ABullet::StaticClass();

	MuzzleAttachPoint = "MuzzleSocket";

	RechargeRate = 5.f;
	WeaponSlotType = WeaponSlotType::Main;
	
	CurrentAmmoItem = nullptr;
	PendingAmmoItem = nullptr;
	PrimaryActorTick.bCanEverTick = true;
}

void ALastimFirearm::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// TEST
	if (Role >= ROLE_Authority)
	{
		if (DefaultAmmoClass)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ALastimAmmo* NewItem = GetWorld()->SpawnActor<ALastimAmmo>(DefaultAmmoClass, SpawnInfo);
			CurrentAmmoItem = NewItem;
		}
	}
	UpdateStatusDisplay();
}

void ALastimFirearm::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateStatusDisplay();
}

/** Input-Related Functions **/

void ALastimFirearm::StartFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}
	
	if (CurrentState == "Reloading" && bContinueReloading)
	{
		bContinueReloading = false;
	}
	else if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void ALastimFirearm::StopFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void ALastimFirearm::StartReload(bool bFromReplication)
{
	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		PendingAmmoItem = ChooseBestAmmoItem();
		if (PendingAmmoItem)
		{
			if (MyPawn)
			{
				MyPawn->RemoveFromInventory(PendingAmmoItem);
			}
			bPendingReload = true;
			bContinueReloading = true;
			DetermineWeaponState();

			float ReloadTime = PlayWeaponAnimation(ReloadAnim1P, ReloadAnim3P);
			if (ReloadTime <= 0.0f)
			{
				ReloadTime = FirearmConfig.ReloadDuration;
			}

			GetWorldTimerManager().SetTimer(TimerHandle_FinishReload, this, &ALastimFirearm::FinishReload, ReloadTime, false);
			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_ReloadFirearm, this, &ALastimFirearm::ReloadFirearm, ReloadTime - 0.05f, false);
			}
		}
	}
}

ALastimAmmo* ALastimFirearm::ChooseBestAmmoItem()
{
	ALastimAmmo* BestAmmo = nullptr;
	for (int32 i = 0; i < MyPawn->ItemInventory.Num(); i++)
	{
		ALastimAmmo* TestAmmo = Cast<ALastimAmmo>(MyPawn->ItemInventory[i]);
		if (TestAmmo && TestAmmo->GetClass() == DefaultAmmoClass)
		{
			if (!CurrentAmmoItem || CurrentAmmoItem->GetAmmoCount() < TestAmmo->GetAmmoCount())
			{
				BestAmmo = TestAmmo;
			}
		}
	}
	return BestAmmo;
}

void ALastimFirearm::FinishReload()
{
	if (CurrentState == "Reloading")
	{
		bPendingReload = false;
		DetermineWeaponState();
		//StopWeaponAnimation(ReloadAnim);
	}
}

void ALastimFirearm::ReloadFirearm()
{
	if (DefaultAmmoClass && MyPawn)
	{
		// TEMP: Create one if we did not find one.
		/*if (!PendingAmmoItem)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ALastimAmmo* NewItem = GetWorld()->SpawnActor<ALastimAmmo>(DefaultAmmoClass, SpawnInfo);
			PendingAmmoItem = NewItem;
		}*/
		////////////////////////////////
		// ACTUAL RELOAD CODE
		////////////////////////////////
		if (PendingAmmoItem)
		{
			MyPawn->RemoveFromInventory(PendingAmmoItem);
			ALastimAmmo* OldAmmoItem = CurrentAmmoItem;
			CurrentAmmoItem = PendingAmmoItem;
			PendingAmmoItem = nullptr;

			if (CurrentAmmoItem)
			{
				CurrentAmmoItem->OnLoadIntoWeapon();
			}
			if (OldAmmoItem)
			{
				MyPawn->AddToInventory(OldAmmoItem);
				OldAmmoItem->OnUnloadFromWeapon();
			}
		}
	}
}


void ALastimFirearm::CancelReloadInProgress()
{
	// Cancel timers.
	GetWorldTimerManager().ClearTimer(TimerHandle_FinishReload);
	GetWorldTimerManager().ClearTimer(TimerHandle_ReloadFirearm);
	// Reinsert the pending ammo item into the player's inventory.
	if (PendingAmmoItem && MyPawn && MyPawn->IsAlive())
	{
		MyPawn->AddToInventory(PendingAmmoItem);
	}
	PendingAmmoItem = nullptr;
	FinishReload();
}

void ALastimFirearm::StartSwitchFireMode(bool bFromReplication)
{
	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartSwitchFireMode();
	}

	if (bFromReplication || CanSwitchFireMode())
	{
		bPendingSwitchFireMode = true;
		DetermineWeaponState();

		float SFMDuration = PlayWeaponAnimation(SwitchFireModeAnim1P, SwitchFireModeAnim3P);
		if (SFMDuration <= 0.0f)
		{
			SFMDuration = 0.25f;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_FinishSwitchFireMode, this, &ALastimFirearm::FinishSwitchFireMode, SFMDuration, false);
		if (Role == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ChangeFireMode, this, &ALastimFirearm::ChangeFireMode, FMath::Max(0.1f, SFMDuration - 0.05f), false);
		}
		/**
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
		PlayWeaponSound(ReloadSound);
		}
		**/
	}
}

void ALastimFirearm::FinishSwitchFireMode()
{
	if (CurrentState == "SwitchingFireMode")
	{
		bPendingSwitchFireMode = false;
		DetermineWeaponState();
		//StopWeaponAnimation(ReloadAnim);
	}
}

void ALastimFirearm::ChangeFireMode()
{
	CurrentFireMode++;
	if (CurrentFireMode >= MaxFireModes)
	{
		CurrentFireMode = 0;
	}
}

/** Boolean Functions **/

bool ALastimFirearm::CanFire()
{
	bool bStateOKToFire = ((CurrentState == "Idle") || (CurrentState == "Firing"));
	return !bPendingReload && !bPendingSwitchFireMode && bIsEquipped && bStateOKToFire;
}

bool ALastimFirearm::CanReload()
{
	bool bStateOKToReload = ( (CurrentState == "Idle") || (CurrentState == "Firing") );
	bool bAmmoAllowsReload = true; //PendingAmmoItem;
	return bIsEquipped && bAmmoAllowsReload && bStateOKToReload;
}

bool ALastimFirearm::CanSwitchFireMode()
{
	bool bStateOKToReload = ((CurrentState == "Idle") || (CurrentState == "Firing"));
	return bIsEquipped && MaxFireModes > 1 && bStateOKToReload;
}

/** State-Related Functions **/

void ALastimFirearm::SetWeaponState(FName NewState)
{
	const FName PrevState = CurrentState;

	if (PrevState == "Firing" && NewState != "Firing")
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != "Firing" && NewState == "Firing")
	{
		OnBurstStarted();
	}
}

void ALastimFirearm::DetermineWeaponState()
{
	FName NewState = "Idle";
	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false)
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = "Reloading";
			}
		}
		else if (bPendingSwitchFireMode)
		{
			if (CanSwitchFireMode() == false)
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = "SwitchingFireMode";
			}
		}
		else if ((bPendingReload == false) && (bPendingSwitchFireMode == false) && (bWantsToFire == true) && (CanFire() == true))
		{
			NewState = "Firing";
		}
	}
	else if (bPendingEquip)
	{
		NewState = "Equipping";
	}
	else if (bPendingUnequip)
	{
		NewState = "Unequipping";
	}
	SetWeaponState(NewState);
}

void ALastimFirearm::OnBurstStarted()
{
	/** Start firing, or start firing after TimeBetweenShots. **/
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && FireMode[CurrentFireMode].TimeBetweenShots > 0.0f && LastFireTime + FireMode[CurrentFireMode].TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(FTimerHandle_ShotTimer, this, &ALastimFirearm::HandleFiring, LastFireTime + FireMode[CurrentFireMode].TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void ALastimFirearm::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	// stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}
	GetWorldTimerManager().ClearTimer(FTimerHandle_ShotTimer);
	bRefiring = false;
}

void ALastimFirearm::FireWeapon(float DamagePct)
{
	int32 ProjCount = 1;
	if (ProjectileCount.IsValidIndex(CurrentFireMode))
	{
		ProjCount = ProjectileCount[CurrentFireMode];
	}
	
	for (int32 i = 0; i < ProjCount; i++)
	{
		FVector ShootDir = GetAdjustedAimRot().Vector();
		// Temporary fix to prevent projectiles from spawning on top of each other.
		// This makes them spawn in a line, which isn't ideal. They also will still collide with each other.
		FVector ProjectileOffset = ShootDir.Rotation().RotateVector(i * FVector(0.0, 0.0, 4.0f));
		FVector Origin = GetAdjustedAimLoc() + ProjectileOffset;
		ServerFireProjectile(Origin, ShootDir, DamagePct);

		// USE THIS BELOW TO STOP BULLETS COLLIDING IN AIR!
		//PrimitiveComp->MoveIgnoreActors.Add(Instigator);
	}
}

bool ALastimFirearm::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar)
{
	return true;
}

void ALastimFirearm::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar)
{
	FTransform SpawnTM(GetAdjustedAimRot(), Origin); //(ShootDir.Rotation(), JestVector);
	TSubclassOf<ALastimProjectile> ProjClass;
	if (FireMode.IsValidIndex(CurrentFireMode))
	{
		if (ProjectileClass.IsValidIndex(CurrentFireMode))
		{
			ProjClass = ProjectileClass[CurrentFireMode]; //FireMode[CurrentFireMode].ProjectileClass;

			ALastimProjectile* Projectile = Cast<ALastimProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjClass, SpawnTM));
			if (Projectile && CurrentFireMode < FireMode.Num())
			{
				Projectile->Instigator = Instigator;
				Projectile->SetOwner(this);
				Projectile->ImpactDamage = FireMode[CurrentFireMode].ShotDamage * DamageScalar;

				ABullet* BulletProj = Cast<ABullet>(Projectile);
				if (BulletProj)
				{
					BulletProj->SetBulletProperties(FireMode[CurrentFireMode].BulletProps);
				}
				if (MyPawn)
				{
					//FVector OutVel = MyPawn->GetVelocity();
					//Projectile->InitVelocity(OutVel);
					FVector CollPoint, Point = Projectile->GetActorLocation();
					ALastimCharacter* TestChar = Cast<ALastimCharacter>(Instigator);
					bool bOverlapsOwner = 0.f == TestChar->GetCapsuleComponent()->GetDistanceToCollision(Point, CollPoint);
					/*
					if (bOverlapsOwner)
					{
						GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("Weapon overlap detected!")));
						FVector Nudge = GetAdjustedAimRot().Vector() * 10.f;
						SpawnTM += FTransform(FRotator::ZeroRotator, Nudge);
					}*/
				}

				UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
			}
		}
	}
}

void ALastimFirearm::OnEnterInventory(ALastimCharacter* NewOwner)
{
	Super::OnEnterInventory(NewOwner);
	if (MyPawn)
	{
		for (int32 i = 0; i < AmmoInventory.Num(); i++)
		{
			if (AmmoInventory[i] != nullptr)
			{
				MyPawn->AddToInventory(AmmoInventory[i]);
			}
		}
		// We're assuming that all the ammo was given to the pawn.
		AmmoInventory.Empty();
	}
}

void ALastimFirearm::OnLeaveInventory()
{
	Super::OnLeaveInventory();
	if (CurrentState == "Reloading")
	{
		CancelReloadInProgress();
	}
	bWantsToFire = false;
	bPendingReload = false;
}

int32 ALastimFirearm::UseAmmo(int32 Amount)
{
	if (CurrentAmmoItem)
	{
		return CurrentAmmoItem->UseAmmo(Amount);
	}
	return 0;
}

void ALastimFirearm::AddRecoil()
{
	ServerAddRecoilToPawn();
}

bool ALastimFirearm::ServerAddRecoilToPawn_Validate()
{
	return true;
}

void ALastimFirearm::ServerAddRecoilToPawn_Implementation()
{
	const float RandAngle = FMath::DegreesToRadians(FMath::FRandRange(-30.0f, 30.f));
	/* Note: X is Pitch and Y is Yaw. If we move to Vector2D, we want to swap these. */
	FVector TestVector = FVector(FMath::Cos(RandAngle), FMath::Sin(RandAngle), 0.0f) * RecoilPerShot;
	MyPawn->AddRecoil(TestVector);
}

FRotator ALastimFirearm::GetAdjustedAimRot() const
{
	/** Add spread if weapon has it, else return base implementation. 
	    Remember, recoil offset is handled in the pawn's code. This is just the spread inaccuracy. */
	if (SpreadRadius > 0 && SpreadRange > 0)
	{
		const int32 RandomSeed = FMath::Rand();
		FRandomStream WeaponRandomStream(RandomSeed);
		const float ConeHalfAngle = FMath::Atan(SpreadRadius / (SpreadRange * 100));
		const FVector ShootDir = WeaponRandomStream.VRandCone(FVector::ForwardVector, ConeHalfAngle, ConeHalfAngle);
		const FRotator Spread = FRotator::ZeroRotator + ShootDir.Rotation();
		return Spread + Super::GetAdjustedAimRot();
	}
	else
	{
		return Super::GetAdjustedAimRot();
	}
}

void ALastimFirearm::HandleFiring()
{
	if (GetAmmo() >= FireMode[CurrentFireMode].AmmoPerShot && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			UseAmmo(FireMode[CurrentFireMode].AmmoPerShot);
			FireWeapon();
			AddRecoil();

			// update firing FX on remote clients if function was called on server
			BurstCounter++;
		}
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetAmmo() <= 0 && !bRefiring && OutOfAmmoSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, OutOfAmmoSound, GetActorLocation());
		}

		// stop weapon fire FX, but stay in Firing state
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		} 
	}
	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// Setup refire timer.
		bRefiring = (CurrentState == "Firing" && FireMode[CurrentFireMode].TimeBetweenShots > 0.0f &&
			        (FireMode[CurrentFireMode].ShotsPerBurst == 0 || FireMode[CurrentFireMode].ShotsPerBurst > BurstCounter));
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(FTimerHandle_ShotTimer, this, &ALastimFirearm::HandleFiring, FireMode[CurrentFireMode].TimeBetweenShots, false);
		}
	}
	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool ALastimFirearm::ServerHandleFiring_Validate()
{
	return true;
}

void ALastimFirearm::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (GetAmmo() > FireMode[CurrentFireMode].AmmoPerShot && CanFire());

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		// update Ammo
		if (CurrentAmmoItem != nullptr)
		{
			float TestAmmoAmount = FireMode.Num() > CurrentFireMode ? FireMode[CurrentFireMode].AmmoPerShot : 0.8f;
			UseAmmo(TestAmmoAmount);
		}

		// update firing FX on remote clients
		BurstCounter++;
	}
}

void ALastimFirearm::SimulateWeaponFire()
{
	if (Role == ROLE_Authority && CurrentState != "Firing")
	{
		return;
	}

	if (FireFX)
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		//if ( !bLoopedMuzzleFX || FiringPSC == NULL)
		{
			// Split screen requires we create 2 effects. One that we see and one that the other player sees.
			// But ShooterGame had split screen, not Lastim, so fuck it for now.
			/**
			if ((MyPawn != NULL) && (MyPawn->IsLocallyControlled() == true))
			{
				AController* PlayerCon = MyPawn->GetController();
				if (PlayerCon != NULL)
				{
					Mesh1P->GetSocketLocation(MuzzleAttachPoint);
					FirePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh1P, MuzzleAttachPoint);
					FirePSC->bOwnerNoSee = false;
					FirePSC->bOnlyOwnerSee = true;

					Mesh3P->GetSocketLocation(MuzzleAttachPoint);
					FirePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, Mesh3P, MuzzleAttachPoint);
					FirePSC->bOwnerNoSee = true;
					FirePSC->bOnlyOwnerSee = false;
				}
			}
			**/
			//else
			{
				FirePSC = UGameplayStatics::SpawnEmitterAttached(FireFX, UseWeaponMesh, MuzzleAttachPoint);
			}
		}
	}

	if (FireSound)
	{
		UGameplayStatics::SpawnSoundAttached(FireSound, MyPawn->GetRootComponent()); //, MuzzleAttachPoint);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("A %s is simulating fire!"), *this->DisplayName.ToString()));

}

void ALastimFirearm::StopSimulatingWeaponFire()
{
	// Null for now.
}

void ALastimFirearm::UpdateStatusDisplay()
{
	for (int32 i = 0; i < MeshMIDs.Num(); i++)
	{
		if (MeshMIDs[i]->IsValidLowLevel())
		{
			MeshMIDs[i]->SetScalarParameterValue(TEXT("EnergyPct"), GetAmmoPct());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// AI

ALastimProjectile * ALastimFirearm::GetModifiedProjectile(int32 FireModeNum)
{
	if (ProjectileClass.IsValidIndex(FireModeNum))
	{
		ALastimProjectile* Proj = ProjectileClass[FireModeNum].GetDefaultObject();
		if (Proj)
		{
			ABullet* BulletProj = Cast<ABullet>(Proj);
			if (BulletProj && FireMode.IsValidIndex(FireModeNum))
			{
				BulletProj->SetBulletProperties(FireMode[FireModeNum].BulletProps);
			}
			return Proj;
		}
	}
	return nullptr;
}

bool ALastimFirearm::IsFullAuto()
{
	return FireMode[CurrentFireMode].ShotsPerBurst == 0;
}

bool ALastimFirearm::ShouldReload()
{
	// Reload if the weapon has less than 50% Ammo.
	if (CanReload() && GetMaxAmmo() * 0.5f > GetAmmo())
	{
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ALastimFirearm::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ALastimFirearm, CurrentAmmoItem, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ALastimFirearm, PendingAmmoItem, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(ALastimFirearm, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ALastimFirearm, bPendingReload, COND_SkipOwner);
}

void ALastimFirearm::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

void ALastimFirearm::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload(true);
	}
	else
	{
		FinishReload();
	}
}

void ALastimFirearm::OnRep_SwitchFireMode()
{
	if (bPendingSwitchFireMode)
	{
		StartSwitchFireMode(true);
	}
	else
	{
		FinishSwitchFireMode();
	}
}

int32 ALastimFirearm::GetAmmo() const
{
	if (CurrentAmmoItem)
	{
		return CurrentAmmoItem->GetAmmoCount();
	}
	return 0;
}

int32 ALastimFirearm::GetMaxAmmo() const
{
	if (CurrentAmmoItem)
	{
		return CurrentAmmoItem->GetMaxAmmo();
	}
	return 0;
}

float ALastimFirearm::GetAmmoPct() const
{
	if (GetMaxAmmo() > 0)
	{
		return (float)GetAmmo() / (float)GetMaxAmmo();
	}
	else
	{
		return 0.0f;
	}
}

int32 ALastimFirearm::GetAmmoForFireMode(int32 Num) const
{
	if (Num < FireMode.Num())
	{
		return GetAmmo() / FireMode[Num].AmmoPerShot;
	}
	return 0;
}

int32 ALastimFirearm::GetMaxAmmoForFireMode(int32 Num) const
{
	if (Num < FireMode.Num())
	{
		return GetMaxAmmo() / FireMode[Num].AmmoPerShot;
	}
	return 0;
}

int32 ALastimFirearm::GetCurrentFireMode() const
{
	return CurrentFireMode;
}

float ALastimFirearm::GetAimSpeed()
{
	return FirearmConfig.AimSpeed;
}

FVector ALastimFirearm::GetAimedOffset() const
{
	return AimedOffset;
}