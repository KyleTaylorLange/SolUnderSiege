// Fill out your copyright notice in the Description page of Project Settings.

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "Projectile.h"
#include "Bullet.h" // To make this the default projectile.
#include "Ammo.h"
#include "SolCharacter.h"
#include "Firearm.h"

//////////////////////////////////////////////////////////////////////////
// AFirearm

AFirearm::AFirearm(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

	// TEMP
	bUseInternalAmmo = false;
	ReserveAmmoScalar = 4;
	RechargeTime = 5;
}

void AFirearm::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// TEST
	if (GetLocalRole() >= ROLE_Authority)
	{
		if (DefaultAmmoClass)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AAmmo* NewItem = GetWorld()->SpawnActor<AAmmo>(DefaultAmmoClass, SpawnInfo);
			CurrentAmmoItem = NewItem;
			if (CurrentAmmoItem) {
				MaxAmmo = CurrentAmmoItem->GetMaxAmmo();
				Ammo = MaxAmmo;
				ReserveAmmo = MaxAmmo * ReserveAmmoScalar;
			}
		}
	}
	UpdateStatusDisplay();
}

void AFirearm::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateStatusDisplay();
}

/** Input-Related Functions **/

void AFirearm::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
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

void AFirearm::StopFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void AFirearm::StartReload(bool bFromReplication)
{
	if (!bFromReplication && GetLocalRole() < ROLE_Authority)
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

			GetWorldTimerManager().SetTimer(TimerHandle_FinishReload, this, &AFirearm::FinishReload, ReloadTime, false);
			if (GetLocalRole() == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_ReloadFirearm, this, &AFirearm::ReloadFirearm, ReloadTime - 0.05f, false);
			}
		}
	}
}

AAmmo* AFirearm::ChooseBestAmmoItem()
{
	AAmmo* BestAmmo = nullptr;
	for (int32 i = 0; i < MyPawn->ItemInventory.Num(); i++)
	{
		AAmmo* TestAmmo = Cast<AAmmo>(MyPawn->ItemInventory[i]);
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

void AFirearm::FinishReload()
{
	if (CurrentState == "Reloading")
	{
		bPendingReload = false;
		DetermineWeaponState();
		//StopWeaponAnimation(ReloadAnim);
	}
}

void AFirearm::ReloadFirearm()
{
	if (bUseInternalAmmo) 
	{
		int32 TransferredAmmo = FMath::Min(ReserveAmmo, MaxAmmo - Ammo);
		Ammo += TransferredAmmo;
		ReserveAmmo -= TransferredAmmo;
	}
	else if (DefaultAmmoClass && MyPawn)
	{
		// TEMP: Create one if we did not find one.
		/*if (!PendingAmmoItem)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AAmmo* NewItem = GetWorld()->SpawnActor<AAmmo>(DefaultAmmoClass, SpawnInfo);
			PendingAmmoItem = NewItem;
		}*/
		////////////////////////////////
		// ACTUAL RELOAD CODE
		////////////////////////////////
		if (PendingAmmoItem)
		{
			MyPawn->RemoveFromInventory(PendingAmmoItem);
			AAmmo* OldAmmoItem = CurrentAmmoItem;
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


void AFirearm::CancelReloadInProgress()
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

void AFirearm::StartSwitchFireMode(bool bFromReplication)
{
	if (!bFromReplication && GetLocalRole() < ROLE_Authority)
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

		GetWorldTimerManager().SetTimer(TimerHandle_FinishSwitchFireMode, this, &AFirearm::FinishSwitchFireMode, SFMDuration, false);
		if (GetLocalRole() == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ChangeFireMode, this, &AFirearm::ChangeFireMode, FMath::Max(0.1f, SFMDuration - 0.05f), false);
		}
		/**
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
		PlayWeaponSound(ReloadSound);
		}
		**/
	}
}

void AFirearm::FinishSwitchFireMode()
{
	if (CurrentState == "SwitchingFireMode")
	{
		bPendingSwitchFireMode = false;
		DetermineWeaponState();
		//StopWeaponAnimation(ReloadAnim);
	}
}

void AFirearm::ChangeFireMode()
{
	CurrentFireMode++;
	if (CurrentFireMode >= MaxFireModes)
	{
		CurrentFireMode = 0;
	}
}

/** Boolean Functions **/

bool AFirearm::CanFire()
{
	bool bStateOKToFire = ((CurrentState == "Idle") || (CurrentState == "Firing"));
	return !bPendingReload && !bPendingSwitchFireMode && bIsEquipped && bStateOKToFire;
}

bool AFirearm::CanReload()
{
	if (bUseInternalAmmo) {
		return false;
	}
	else {
		bool bStateOKToReload = ((CurrentState == "Idle") || (CurrentState == "Firing"));
		bool bAmmoAllowsReload = true; //PendingAmmoItem;
		return bIsEquipped && bAmmoAllowsReload && bStateOKToReload;
	}
}

bool AFirearm::CanSwitchFireMode()
{
	bool bStateOKToReload = ((CurrentState == "Idle") || (CurrentState == "Firing"));
	return bIsEquipped && MaxFireModes > 1 && bStateOKToReload;
}

/** State-Related Functions **/

void AFirearm::SetWeaponState(FName NewState)
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

void AFirearm::DetermineWeaponState()
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

void AFirearm::OnBurstStarted()
{
	/** Start firing, or start firing after TimeBetweenShots. **/
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && FireMode[CurrentFireMode].TimeBetweenShots > 0.0f && LastFireTime + FireMode[CurrentFireMode].TimeBetweenShots > GameTime)
	{
		GetWorldTimerManager().SetTimer(FTimerHandle_ShotTimer, this, &AFirearm::HandleFiring, LastFireTime + FireMode[CurrentFireMode].TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AFirearm::OnBurstFinished()
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

void AFirearm::FireWeapon(float DamagePct)
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

bool AFirearm::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar)
{
	return true;
}

void AFirearm::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar)
{
	FTransform SpawnTM(GetAdjustedAimRot(), Origin); //(ShootDir.Rotation(), JestVector);
	TSubclassOf<AProjectile> ProjClass;
	if (FireMode.IsValidIndex(CurrentFireMode))
	{
		if (ProjectileClass.IsValidIndex(CurrentFireMode))
		{
			ProjClass = ProjectileClass[CurrentFireMode]; //FireMode[CurrentFireMode].ProjectileClass;

			AProjectile* Projectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjClass, SpawnTM));
			if (Projectile && CurrentFireMode < FireMode.Num())
			{
				Projectile->SetInstigator(GetInstigator());
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
					ASolCharacter* TestChar = Cast<ASolCharacter>(GetInstigator());
					bool bOverlapsOwner = 0.f == TestChar->GetCapsuleComponent()->GetDistanceToCollision(Point, CollPoint);
					/*
					if (bOverlapsOwner)
					{
						//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("Weapon overlap detected!")));
						FVector Nudge = GetAdjustedAimRot().Vector() * 10.f;
						SpawnTM += FTransform(FRotator::ZeroRotator, Nudge);
					}*/
				}

				UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
			}
		}
	}
}

void AFirearm::OnEnterInventory(ASolCharacter* NewOwner)
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

void AFirearm::OnLeaveInventory()
{
	Super::OnLeaveInventory();
	if (CurrentState == "Reloading")
	{
		CancelReloadInProgress();
	}
	bWantsToFire = false;
	bPendingReload = false;
}

int32 AFirearm::UseAmmo(int32 Amount)
{
	if (bUseInternalAmmo) {
		int32 UsedAmmo = FMath::Min(Ammo, Amount);
		Ammo -= UsedAmmo;
		return UsedAmmo;
	}
	if (CurrentAmmoItem)
	{
		return CurrentAmmoItem->UseAmmo(Amount);
	}
	return 0;
}

void AFirearm::AddRecoil()
{
	ServerAddRecoilToPawn();
}

bool AFirearm::ServerAddRecoilToPawn_Validate()
{
	return true;
}

void AFirearm::ServerAddRecoilToPawn_Implementation()
{
	const float RandAngle = FMath::DegreesToRadians(FMath::FRandRange(-30.0f, 30.f));
	/* Note: X is Pitch and Y is Yaw. If we move to Vector2D, we want to swap these. */
	FVector TestVector = FVector(FMath::Cos(RandAngle), FMath::Sin(RandAngle), 0.0f) * RecoilPerShot;
	MyPawn->AddRecoil(TestVector);
}

FRotator AFirearm::GetAdjustedAimRot() const
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

void AFirearm::HandleFiring()
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
			if (bUseInternalAmmo) {
				GetWorldTimerManager().SetTimer(FTimerHandle_RechargeIntAmmo, this, &AFirearm::ReloadFirearm, RechargeTime, false);
			}

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
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// Setup refire timer.
		bRefiring = (CurrentState == "Firing" && FireMode[CurrentFireMode].TimeBetweenShots > 0.0f &&
			        (FireMode[CurrentFireMode].ShotsPerBurst == 0 || FireMode[CurrentFireMode].ShotsPerBurst > BurstCounter));
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(FTimerHandle_ShotTimer, this, &AFirearm::HandleFiring, FireMode[CurrentFireMode].TimeBetweenShots, false);
		}
	}
	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool AFirearm::ServerHandleFiring_Validate()
{
	return true;
}

void AFirearm::ServerHandleFiring_Implementation()
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

void AFirearm::SimulateWeaponFire()
{
	if (GetLocalRole() == ROLE_Authority && CurrentState != "Firing")
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
	////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("A %s is simulating fire!"), *this->DisplayName.ToString()));

}

void AFirearm::StopSimulatingWeaponFire()
{
	// Null for now.
}

void AFirearm::UpdateStatusDisplay()
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

AProjectile * AFirearm::GetModifiedProjectile(int32 FireModeNum)
{
	if (ProjectileClass.IsValidIndex(FireModeNum))
	{
		AProjectile* Proj = ProjectileClass[FireModeNum].GetDefaultObject();
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

bool AFirearm::IsFullAuto()
{
	return FireMode[CurrentFireMode].ShotsPerBurst == 0;
}

bool AFirearm::ShouldReload()
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

void AFirearm::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFirearm, CurrentAmmoItem, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFirearm, PendingAmmoItem, COND_OwnerOnly);

	DOREPLIFETIME_CONDITION(AFirearm, BurstCounter, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AFirearm, bPendingReload, COND_SkipOwner);
}

void AFirearm::OnRep_BurstCounter()
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

void AFirearm::OnRep_Reload()
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

void AFirearm::OnRep_SwitchFireMode()
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

int32 AFirearm::GetAmmo() const
{
	if (bUseInternalAmmo) {
		return Ammo;
	}
	else if (CurrentAmmoItem)
	{
		return CurrentAmmoItem->GetAmmoCount();
	}
	return 0;
}

int32 AFirearm::GetMaxAmmo() const
{
	if (bUseInternalAmmo) {
		return MaxAmmo;
	}
	else if (CurrentAmmoItem)
	{
		return CurrentAmmoItem->GetMaxAmmo();
	}
	return 0;
}

float AFirearm::GetAmmoPct() const
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

int32 AFirearm::GetAmmoForFireMode(int32 Num) const
{
	if (Num < FireMode.Num())
	{
		return GetAmmo() / FireMode[Num].AmmoPerShot;
	}
	return 0;
}

int32 AFirearm::GetMaxAmmoForFireMode(int32 Num) const
{
	if (Num < FireMode.Num())
	{
		return GetMaxAmmo() / FireMode[Num].AmmoPerShot;
	}
	return 0;
}


int32 AFirearm::GetReserveAmmoForFireMode(int32 Num) const
{
	if (Num < FireMode.Num())
	{
		return ReserveAmmo / FireMode[Num].AmmoPerShot;
	}
	return 0;
}

int32 AFirearm::GetCurrentFireMode() const
{
	return CurrentFireMode;
}

float AFirearm::GetAimSpeed()
{
	return FirearmConfig.AimSpeed;
}

FVector AFirearm::GetAimedOffset() const
{
	return AimedOffset;
}