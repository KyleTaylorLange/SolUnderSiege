// Fill out your copyright notice in the Description page of Project Settings.

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "Projectile.h"
#include "Bullet.h" // To make this the default projectile.
#include "Ammo.h"
#include "SolCharacter.h"
#include "SolDamageType.h"
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

	ProjectileClass.SetNum(1);
	ProjectileClass[0] = ABullet::StaticClass();

	MuzzleAttachPoint = "MuzzleSocket";

	WeaponSlotType = WeaponSlotType::Main;
	
	CurrentAmmoItem.Add(nullptr);
	PendingAmmoItem.Add(nullptr);
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UFont> FontObj(TEXT("/Game/UI/HUD/Roboto51"));
	ScreenDisplayFont = FontObj.Object;

	MaxHeat = 1;
	Heat = 0;
	CooldownDelay = 0;

	/* Test scope code to be moved to a future attachment class:
	//IN CONSTRUCTOR:
	SceneCapComp = ObjectInitializer.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("TestSceneCapComp"));
	SceneCapComp->AttachTo(GetMesh3P());
	SceneCapComp->FOVAngle = 4.5f;
	// In PostInitializeComponents()
	SceneCapComp->TextureTarget = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass());
	SceneCapComp->TextureTarget->InitAutoFormat(512, 512);
	SceneCapComp->SetVisibility(true);
	// IN SECTION UPDATING MIDs:
	if (SceneCapComp->TextureTarget)
	{
		MeshMIDs[i]->SetTextureParameterValue(TEXT("ScreenTexture"), SceneCapComp->TextureTarget);
	}
	*/
}

void AFirearm::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// TEST
	if (GetLocalRole() >= ROLE_Authority)
	{
		CurrentAmmoItem.SetNum(DefaultAmmoClass.Num());
		PendingAmmoItem.SetNum(DefaultAmmoClass.Num());
		for (int i = 0; i < DefaultAmmoClass.Num(); i++)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AAmmo* NewItem = GetWorld()->SpawnActor<AAmmo>(DefaultAmmoClass[i], SpawnInfo);
			CurrentAmmoItem[i] = NewItem;
			if (CurrentAmmoItem[i])
			{
				CurrentAmmoItem[i]->OnLoadIntoWeapon();
				MaxHeat += CurrentAmmoItem[i]->GetMaxAmmo();
			}
		}
	}
	ScreenRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(GEngine->GetWorld(), UCanvasRenderTarget2D::StaticClass(), 256, 256);
	ScreenRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AFirearm::DrawCanvasStatusDisplayElements);
	UpdateStatusDisplay();
}

void AFirearm::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (CooldownDelay > 0)
	{
		CooldownDelay = FMath::Max(0.f, CooldownDelay - DeltaSeconds);
	}
	else if (CooldownDelay <= 0 && Heat > 0)
	{
		Heat = FMath::Max(0.f, Heat - (MaxHeat * DeltaSeconds / 5.f));
	}
	UpdateStatusDisplay();
}

/** Input-Related Functions **/
void AFirearm::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}
	
	if (!bWantsToFire)
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
		// Reload first slot that needs reloading.
		int32 SlotNum = -1;
		for (int32 i = 0; i < CurrentAmmoItem.Num(); i++)
		{
			if (CurrentAmmoItem[i] && CurrentAmmoItem[i]->GetAmmoCount() < CurrentAmmoItem[i]->GetMaxAmmo())
			{
				SlotNum = i;
				break;
			}
		}

		if (SlotNum != -1)
		{
			PendingAmmoItem[SlotNum] = ChooseBestAmmoItem(SlotNum);
			if (PendingAmmoItem[SlotNum])
			{
				if (MyPawn)
				{
					MyPawn->RemoveFromInventory(PendingAmmoItem[SlotNum]);
				}
				bPendingReload = true;
				DetermineWeaponState();

				float ReloadTime = PlayWeaponAnimation(ReloadAnim1P, ReloadAnim3P);
				if (ReloadTime <= 0.0f)
				{
					ReloadTime = FirearmConfig.ReloadDuration;
				}

				GetWorldTimerManager().SetTimer(TimerHandle_FinishReload, this, &AFirearm::FinishReload, ReloadTime, false);
				if (GetLocalRole() == ROLE_Authority)
				{
					TimerDel_ReloadFirearm.BindUFunction(this, FName("ReloadFirearm"), SlotNum);
					//GetWorldTimerManager().SetTimer(TimerDel_ReloadFirearm, this, &AFirearm::ReloadFirearm, ReloadTime - 0.05f, false);
					GetWorldTimerManager().SetTimer(TimerHandle_ReloadFirearm, TimerDel_ReloadFirearm, ReloadTime - 0.05f, false);
				}
			}
		}
	}
}

AAmmo* AFirearm::ChooseBestAmmoItem(int SlotIndex)
{
	AAmmo* BestAmmo = nullptr;
	for (int32 i = 0; i < MyPawn->ItemInventory.Num(); i++)
	{
		AAmmo* TestAmmo = Cast<AAmmo>(MyPawn->ItemInventory[i]);
		if (TestAmmo && TestAmmo->GetClass() == DefaultAmmoClass[SlotIndex])
		{
			if (!CurrentAmmoItem[SlotIndex] || CurrentAmmoItem[SlotIndex]->GetAmmoCount() < TestAmmo->GetAmmoCount())
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

void AFirearm::ReloadFirearm(int SlotIndex)
{
	if (DefaultAmmoClass[SlotIndex] && MyPawn)
	{
		if (PendingAmmoItem[SlotIndex])
		{
			if (UseSimplifiedAmmo())
			{
				int32 AmmoToReload = CurrentAmmoItem[SlotIndex]->GetMaxAmmo() - CurrentAmmoItem[SlotIndex]->GetAmmoCount();
				AmmoToReload = PendingAmmoItem[SlotIndex]->UseAmmo(AmmoToReload);
				CurrentAmmoItem[SlotIndex]->AddAmmo(AmmoToReload);
				AAmmo* OldAmmoItem = PendingAmmoItem[SlotIndex];
				PendingAmmoItem[SlotIndex] = nullptr;
				if (OldAmmoItem->GetAmmoCount() <= 0)
				{
					MyPawn->RemoveFromInventory(OldAmmoItem);
					OldAmmoItem->Destroy();
				}
			}
			else
			{
				MyPawn->RemoveFromInventory(PendingAmmoItem[SlotIndex]);
				AAmmo* OldAmmoItem = CurrentAmmoItem[SlotIndex];
				CurrentAmmoItem[SlotIndex] = PendingAmmoItem[SlotIndex];
				PendingAmmoItem[SlotIndex] = nullptr;

				if (CurrentAmmoItem[SlotIndex])
				{
					CurrentAmmoItem[SlotIndex]->OnLoadIntoWeapon();
				}
				if (OldAmmoItem)
				{
					MyPawn->AddToInventory(OldAmmoItem);
					OldAmmoItem->OnUnloadFromWeapon();
				}
			}
		}
	}
}

void AFirearm::CancelReloadInProgress()
{
	// Cancel timers.
	GetWorldTimerManager().ClearTimer(TimerHandle_FinishReload);
	GetWorldTimerManager().ClearTimer(TimerHandle_ReloadFirearm);
	// Reinsert pending ammo items into the player's inventory.
	if (MyPawn && MyPawn->IsAlive())
	{
		for (int32 i = 0; i < PendingAmmoItem.Num(); i++)
		{
			if (PendingAmmoItem[i])
			{
				MyPawn->AddToInventory(PendingAmmoItem[i]);
			}
		}
	}
	for (int32 i = 0; i < PendingAmmoItem.Num(); i++)
	{
		if (PendingAmmoItem[i])
		{
			PendingAmmoItem[i] = nullptr;
		}
	}
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
	bool bStateOKToReload = ((CurrentState == "Idle") || (CurrentState == "Firing"));
	bool bAmmoAllowsReload = true; //PendingAmmoItem;
	return bIsEquipped && bAmmoAllowsReload && bStateOKToReload;
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
	if (FireMode.IsValidIndex(CurrentFireMode))
	{
		int32 ProjCount = FireMode[CurrentFireMode].ProjectileCount;

		for (int32 i = 0; i < ProjCount; i++)
		{
			// Add spread to our aim rotation.
			FVector ShootDir = (GetAdjustedAimRot() + CalculateSpread()).Vector();
			// Temporary fix to prevent projectiles from spawning on top of each other.
			// This makes them spawn in a line, which isn't ideal. They also will still collide with each other.
			FVector TempProjectileOffset = ShootDir.Rotation().RotateVector(i * FVector(0.0, 0.0, 4.0f));
			FVector Origin = GetAdjustedAimLoc() + TempProjectileOffset;
			if (FireMode[CurrentFireMode].ProjectileClass)
			{
				ServerFireProjectile(Origin, ShootDir, DamagePct);
			}
			else
			{
				const float TEMP_TraceLength = 100000000.f;
				ServerFireHitscan(Origin, ShootDir, TEMP_TraceLength, DamagePct);
			}
		}
	}
}

bool AFirearm::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar)
{
	return true;
}

void AFirearm::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	if (FireMode.IsValidIndex(CurrentFireMode))
	{
		TSubclassOf<AProjectile> ProjClass = FireMode[CurrentFireMode].ProjectileClass;

		AProjectile* Projectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjClass, SpawnTM));
		if (Projectile && FireMode.IsValidIndex(CurrentFireMode))
		{
			Projectile->SetInstigator(GetInstigator());
			Projectile->SetOwner(this);
			Projectile->ImpactDamage = FireMode[CurrentFireMode].ShotDamage * DamageScalar;
			// Temporarily ignore firer so that the projectile does not immediately collide with them.
			// TODO: Nudge projectiles out of firer's collision so they can still collide with firer after firing.
			Projectile->PrimitiveComp->MoveIgnoreActors.Add(GetInstigator());

			ABullet* BulletProj = Cast<ABullet>(Projectile);
			if (BulletProj)
			{
				BulletProj->SetBulletProperties(FireMode[CurrentFireMode].BulletProps);
			}

			UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
			// Inherit owner's velocity after spawning projectile.
			Projectile->InheritVelocity(GetInstigator()->GetMovementComponent()->Velocity);
		}
	}
}

bool AFirearm::ServerFireHitscan_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir, float TraceLength, float DamageScalar = 1.f)
{
	return true;
}

void AFirearm::ServerFireHitscan_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir, float TraceLength, float DamageScalar = 1.f)
{
	if (FireMode.IsValidIndex(CurrentFireMode))
	{
		FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
		FHitResult Hit(ForceInit);
		FVector EndPoint = Origin + ShootDir * TraceLength;
		GetWorld()->LineTraceSingleByChannel(Hit, Origin, EndPoint, COLLISION_PROJECTILE, TraceParams);

		FSolPointDamageEvent PointDmgEvent;
		//PointDmg.DamageTypeClass = InstantConfig.DamageType;
		PointDmgEvent.HitInfo = Hit;
		PointDmgEvent.ShotDirection = ShootDir;
		PointDmgEvent.Damage = FireMode[CurrentFireMode].ShotDamage;

		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("HitInfo: %s"), *Hit.ToString()));
		if (Hit.GetActor())
		{
			Hit.GetActor()->TakeDamage(FireMode[CurrentFireMode].ShotDamage, PointDmgEvent, GetInstigator()->Controller, this);
		}
	}
}

void AFirearm::OnEnterInventory(ASolCharacter* NewOwner)
{
	Super::OnEnterInventory(NewOwner);
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

float AFirearm::UseAmmo(float Amount, int SlotIndex)
{
	if (CurrentAmmoItem[SlotIndex])
	{
		float UsedAmmo = CurrentAmmoItem[SlotIndex]->UseAmmo(Amount);
		Heat += UsedAmmo;
		return UsedAmmo;
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

FRotator AFirearm::CalculateSpread() const
{
	if (SpreadRadius > 0 && SpreadRange > 0)
	{
		const int32 RandomSeed = FMath::Rand();
		FRandomStream WeaponRandomStream(RandomSeed);
		// Calculate the angle based on the radius it should hit at a certain range.
		const float ConeHalfAngle = FMath::Atan(SpreadRadius / (SpreadRange * 100));
		const FVector ShootDir = WeaponRandomStream.VRandCone(FVector::ForwardVector, ConeHalfAngle);
		return ShootDir.Rotation();
	}
	return FRotator::ZeroRotator;
}

void AFirearm::HandleFiring()
{
	if (GetAmmo() >= FireMode[CurrentFireMode].AmmoPerShot && Heat < MaxHeat && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			float AmmoToUse = FireMode[CurrentFireMode].AmmoPerShot;
			int32 AmmoSlot = FireMode[CurrentFireMode].AmmoSlot;
			UseAmmo(AmmoToUse, AmmoSlot);

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
		if (GetLocalRole() < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// Setup refire timer.
		CooldownDelay = FireMode[CurrentFireMode].TimeBetweenShots + 0.0625f;
		bRefiring = (CurrentState == "Firing" && FireMode[CurrentFireMode].TimeBetweenShots > 0.0f &&
			        (FireMode[CurrentFireMode].ShotsPerBurst == 0 || FireMode[CurrentFireMode].ShotsPerBurst > BurstCounter));
		if (bRefiring)
		{
			float HeatMultiplier = Heat >= MaxHeat ? 1.5f : 1.0f;
			GetWorldTimerManager().SetTimer(FTimerHandle_ShotTimer, this, &AFirearm::HandleFiring, FireMode[CurrentFireMode].TimeBetweenShots * HeatMultiplier, false);
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
		if (CurrentAmmoItem[0] != nullptr)
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
		USkeletalMeshComponent* UseWeaponMesh = GetMesh();
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
	if (ScreenRenderTarget)
	{
		ScreenRenderTarget->UpdateResource();
	}

	for (int32 i = 0; i < MeshMIDs.Num(); i++)
	{
		if (MeshMIDs[i])
		{
			MeshMIDs[i]->SetScalarParameterValue(TEXT("EnergyPct"), (float)((float)Heat / (float)MaxHeat)); //GetAmmoPct());
			if (ScreenRenderTarget)
			{
				MeshMIDs[i]->SetTextureParameterValue(TEXT("ScreenTexture"), ScreenRenderTarget);
			}
		}
	}
}

void AFirearm::DrawCanvasStatusDisplayElements(UCanvas* Canvas, int32 Width, int32 Height)
{
	FCanvasTextItem TextItem(FVector2D::ZeroVector, FText::GetEmpty(), ScreenDisplayFont, FColor::White);
	TextItem.bOutlined = true;
	TextItem.OutlineColor = FLinearColor::Black;
	TextItem.Scale = FVector2D(2.f);
	TextItem.bCentreX = true;
	TextItem.bCentreY = true;
	TextItem.Position = FVector2D(Width / 2, Height / 2);
	TextItem.Text = FText::FromString(FString::Printf(TEXT("%d"), GetAmmoForFireMode(GetCurrentFireMode())));
	Canvas->DrawItem(TextItem);
}

//////////////////////////////////////////////////////////////////////////
// AI

AProjectile* AFirearm::GetModifiedProjectile(int32 FireModeNum)
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
	DOREPLIFETIME_CONDITION(AFirearm, CurrentFireMode, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFirearm, bPendingSwitchFireMode, COND_OwnerOnly);

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

int32 AFirearm::GetAmmo(int32 SlotNum) const
{
	if (CurrentAmmoItem.IsValidIndex(SlotNum) && CurrentAmmoItem[SlotNum])
	{
		return CurrentAmmoItem[SlotNum]->GetAmmoCount();
	}
	return 0;
}

int32 AFirearm::GetMaxAmmo(int32 SlotNum) const
{
	if (CurrentAmmoItem.IsValidIndex(SlotNum) && CurrentAmmoItem[SlotNum])
	{
		return CurrentAmmoItem[SlotNum]->GetMaxAmmo();
	}
	return 0;
}

float AFirearm::GetAmmoPct(int32 SlotNum) const
{
	if (GetMaxAmmo(SlotNum) > 0)
	{
		return (float)GetAmmo(SlotNum) / (float)GetMaxAmmo(SlotNum);
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
		return GetAmmo(FireMode[Num].AmmoSlot) / FireMode[Num].AmmoPerShot;
	}
	return 0;
}

int32 AFirearm::GetMaxAmmoForFireMode(int32 Num) const
{
	if (Num < FireMode.Num())
	{
		return GetMaxAmmo(FireMode[Num].AmmoSlot) / FireMode[Num].AmmoPerShot;
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
