;// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h" // For the sphere trace.
#include "SolGameMode.h"
#include "SolPlayerState.h"
#include "TeamState.h"
#include "SolGameState.h"
#include "InventoryItem.h"
#include "Firearm.h"
#include "Ammo.h"
#include "AmmoBag.h"
#include "SolPlayerController.h"
#include "SolHUD.h"
//#include "Pickup.h" // TO LATER MOVE TO Weapon/InventoryItem
#include "SolCharacterMovementComponent.h"
#include "SolCharacter.h"
#include "Animation/AnimInstance.h"

//////////////////////////////////////////////////////////////////////////
// ASolCharacter

ASolCharacter::ASolCharacter(const FObjectInitializer& ObjectInitializer) //: Super(ObjectInitializer)
  : Super(ObjectInitializer.SetDefaultSubobjectClass<USolCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(47.5f, 87.5f); //175cm tall, 112.5cm crouching (56.25cm HH)
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	BaseEyeHeight = 72.5f; // 15 cm from top of head.
	CrouchedEyeHeight = 41.25f;
	USolCharacterMovementComponent* LastimMC = Cast<USolCharacterMovementComponent>(GetMovementComponent());
	if (LastimMC)
	{
		LastimMC->CrouchedHalfHeight = 87.5f; //56.25f;
	}

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Mesh sockets
	HelmetAttachPoint = "HelmetSocket";
	WeaponAttachPoint = "GunSocketTest";

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("CharacterMesh1P"));
	Mesh1P->bOnlyOwnerSee = true;
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->SetupAttachment(GetCapsuleComponent()); // FirstPersonCameraComponent;
	Mesh1P->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	Mesh1P->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;

	// Update the third person mesh.
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -87.5f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	// Create mesh for the helmet. It can be a StaticMesh since the whole thing would move with the head.
	HelmetMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("HelmetMesh"));
	HelmetMesh->bOnlyOwnerSee = false;
	HelmetMesh->bOwnerNoSee = true;
	HelmetMesh->SetCollisionObjectType(ECC_Pawn);
	HelmetMesh->SetupAttachment(GetMesh(), HelmetAttachPoint);

	Health = 100.f;
	MaxHealth = 100.f;
	SevereDamage = 0.f;
	SevereDamageScalar = 0.125f;
	HealthRegenRate = 1.5f;
	HealthTicksPerSec = 8.f;
	Stamina = 10.f;
	MaxEquipment = 5;

	CurrentAimPct = 0.f;
	bIsDying = false;
	bWeaponFiringAllowed = true;

	static ConstructorHelpers::FObjectFinder<USoundCue> PISoundObj(TEXT("/Game/Sounds/SC_PickupItem.SC_PickupItem"));
	PickupItemSound = PISoundObj.Object;
	static ConstructorHelpers::FObjectFinder<USoundCue> UDSoundObj(TEXT("/Game/Sounds/SC_UseDenial.SC_UseDenial"));
	UseDenialSound = UDSoundObj.Object;
	static ConstructorHelpers::FObjectFinder<USoundCue> RespSoundObj(TEXT("/Game/Sounds/SC_PlayerRespawn.SC_PlayerRespawn"));
	RespawnSound = RespSoundObj.Object;
	PrimaryColorOverride = FLinearColor::Transparent;
	SecondaryColorOverride = FLinearColor::Transparent;

	bReplicates = true;

	// Recoil system variables; in constant flux right now. Most intialize as null/zero.
	CurrentRecoilVelocity = FVector::ZeroVector;
	LastRecoilVelocity = FVector::ZeroVector;
	RecoilCurveTime = 0.0f;
	RecoilTimeScalar = 0.5f;

	HeldWeaponOffset = FRotator::ZeroRotator;
	LastControlRotation = FRotator::ZeroRotator;
	MaxFreeAimRadius = 10.f; //5.f;
	MaxRecoilOffsetRadius = 10.f;
}

void ASolCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetLocalRole() == ROLE_Authority)
	{
		// Called by Game Mode now.
		//SpawnInitialInventory();
	}

	// Create material instances so we can manipulate them later.
	for (int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(iMat));
	}
	for (int32 iMat = 0; iMat < HelmetMesh->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(HelmetMesh->CreateAndSetMaterialInstanceDynamic(iMat));
	}

	// Create material instances so we can manipulate them later.
	for (int32 iMat = 0; iMat < Mesh1P->GetNumMaterials(); iMat++)
	{
		Mesh1PMIDs.Add(Mesh1P->CreateAndSetMaterialInstanceDynamic(iMat));
	}
}

void ASolCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (RespawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, GetActorLocation());
	}
}

void ASolCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsDying)
	{
		/** Use stamina if sprinting **/
		if (IsSprinting())
		{
			Stamina -= 1 * DeltaSeconds;
			if (Stamina < 0.f)
			{
				SetSprinting(false);
			}
		}
		/**Regenerate stamina if not sprinting **/
		else if (Stamina < 11.f && !IsSprinting())
		{
			Stamina += 1 * DeltaSeconds;
			if (Stamina > 11.f)
			{
				Stamina = 11.f;
			}
		}

		/** Adjust the weapon's aim percentage. **/
		if (EquippedWeapon) //Since the weapon itself has the aim speed.
		{
			float AimSpeed = 0.5f;
			AFirearm* EquippedFirearm = Cast<AFirearm>(EquippedWeapon);
			if (EquippedFirearm)
			{
				AimSpeed = EquippedFirearm->GetAimSpeed();
			}
			if (CurrentAimPct < 1.f && bIsAiming)
			{
				CurrentAimPct += DeltaSeconds / AimSpeed;
				if (CurrentAimPct > 1.f)
				{
					CurrentAimPct = 1.f;
				}
			}
			else if (CurrentAimPct > 0.f && !bIsAiming)
			{
				CurrentAimPct -= DeltaSeconds / AimSpeed;
				if (CurrentAimPct < 0.f)
				{
					CurrentAimPct = 0.f;
				}
			}
		}

		/** Add recoil to character if necessary. */
		if (RecoilCurveTime > 0.0f)
		{
			ProcessRecoil(DeltaSeconds);
		}
		/*
		AActor* UseItem = GetUsableObject();
		if (UseItem != nullptr)
		{
			// Do stuff here
		}
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASolCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Reload", IE_Pressed, this, &ASolCharacter::OnReload);
	//InputComponent->BindAction("Reload", IE_Released, this, &ASolCharacter::OnStopReload);

	InputComponent->BindAction("SwitchFireMode", IE_Pressed, this, &ASolCharacter::OnSwitchFireMode);

	InputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &ASolCharacter::OnSwitchWeapon);

	InputComponent->BindAction("EquipSidearm", IE_Pressed, this, &ASolCharacter::OnEquipSidearm);
	InputComponent->BindAction("EquipPrimary", IE_Pressed, this, &ASolCharacter::OnEquipPrimary);
	InputComponent->BindAction("EquipSecondary", IE_Pressed, this, &ASolCharacter::OnEquipSecondary);

	InputComponent->BindAction("DropWeapon", IE_Pressed, this, &ASolCharacter::OnDropWeapon);

	InputComponent->BindAxis("MoveForward", this, &ASolCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASolCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analogue joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ASolCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ASolCharacter::LookUpAtRate);
}

/** Below functions currently are the input. This will later be relegated to the PlayerController.
    This will allow us to do actions while dead ("Press [Fire] To Respawn") or to relegate those
	functions to controlled vehicles. **/

void ASolCharacter::StartFire()
{
	if (EquippedWeapon && bWeaponFiringAllowed)
	{
		EquippedWeapon->StartFire();
	}

	// try and play a firing animation if specified
	if(FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if(AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void ASolCharacter::StopFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

void ASolCharacter::OnReload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartReload();
	}
}

void ASolCharacter::OnSwitchFireMode()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartSwitchFireMode();
	}
}

void ASolCharacter::StartAim()
{
	OnStartAim();
}

void ASolCharacter::StopAim()
{
	OnStopAim();
}

void ASolCharacter::OnStartAim()
{
	if (IsSprinting())
	{
		SetSprinting(false);
	}
	SetAiming(true);
}

void ASolCharacter::OnStopAim()
{
	SetAiming(false);
}

void ASolCharacter::StartSprint()
{
	OnStartSprint();
}

void ASolCharacter::StopSprint()
{
	OnStopSprint();
}

void ASolCharacter::OnStartSprint()
{
	if (IsAiming())
	{
		SetAiming(false);
	}
	SetSprinting(true);
}

void ASolCharacter::OnStopSprint()
{
	SetSprinting(false);
}

// Currently identical to OnNextWeapon()
void ASolCharacter::OnSwitchWeapon() //(int8 WeapNum)
{
	EquipWeaponByDeltaIndex(1);
}

void ASolCharacter::OnPrevWeapon()
{
	EquipWeaponByDeltaIndex(-1);
}

void ASolCharacter::OnNextWeapon()
{
	EquipWeaponByDeltaIndex(1);
}

void ASolCharacter::EquipWeaponByDeltaIndex(int32 DeltaIndex)
{
	if (GetInventoryCount() >= 1 && (EquippedWeapon == NULL || EquippedWeapon->GetWeaponState() != "Equipping"))
	{
		const int32 EquippedWeaponIdx = WeaponInventory.IndexOfByKey(EquippedWeapon); //Becomes -1 if there is no equipped weapon.
		AWeapon* NewWeapon = WeaponInventory[FMath::Abs(EquippedWeaponIdx + DeltaIndex) % GetInventoryCount()];
		if (NewWeapon && NewWeapon != EquippedWeapon)
		{
			SetPendingWeapon(NewWeapon); //EquipWeapon(NewWeapon);
		}
	}
}

void ASolCharacter::OnEquipSidearm()
{
	if (SidearmWeapon && SidearmWeapon != EquippedWeapon)
	{
		SetPendingWeapon(SidearmWeapon); //EquipWeapon(SidearmWeapon);
	}
}

void ASolCharacter::OnEquipPrimary()
{
	if (PrimaryWeapon && PrimaryWeapon != EquippedWeapon)
	{
		SetPendingWeapon(PrimaryWeapon); //EquipWeapon(PrimaryWeapon);
	}
}

void ASolCharacter::OnEquipSecondary()
{
	if (SecondaryWeapon && SecondaryWeapon != EquippedWeapon)
	{
		SetPendingWeapon(SecondaryWeapon);  //EquipWeapon(SecondaryWeapon);
	}
}

void ASolCharacter::OnDropWeapon()
{
	if (EquippedWeapon)
	{
		DropInventory(EquippedWeapon);
	}
}

bool ASolCharacter::OnStartUse()
{
	return HandleUse();
}

void ASolCharacter::OnStopUse()
{
	// Nothing yet.
}

/** Below functions deal with the result of the input. **/

void ASolCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			SetEquippedWeapon(Weapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}

bool ASolCharacter::ServerEquipWeapon_Validate(AWeapon* Weapon)
{
	return true;
}

void ASolCharacter::ServerEquipWeapon_Implementation(AWeapon* Weapon)
{
	EquipWeapon(Weapon); //EquipWeapon(Weapon);
}

void ASolCharacter::DropInventory(AInventoryItem* Inv)
{
	if (Inv && Inv->DroppedPickupClass)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			FVector SpawnVector = GetWeaponAimLoc(); //+ (GetWeaponAimRot().RotateVector(FVector(50.f, 0.f, 0.f)));
			FTransform SpawnTM(GetWeaponAimRot(), SpawnVector);
			// Changed DroppedPickup class to SpecificPickup class since physics don't work on the DroppedPickup.
			ASpecificPickup* DroppedPickup = Cast<ASpecificPickup>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, DroppedPickup->StaticClass(), SpawnTM));
			if (DroppedPickup)
			{
				UE_LOG(LogDamage, Warning, TEXT("%s: Dropping item %s."), *GetName(), *Inv->GetName());
				RemoveFromInventory(Inv);
				DroppedPickup->AssignItemToPickup(Inv);
				// Temporarily set the lifespan here.
				DroppedPickup->SetLifeSpan(60.f);
				// TODO: Inherit the player's velocity.
				UGameplayStatics::FinishSpawningActor(DroppedPickup, SpawnTM);
			}
		}
		else
		{
			ServerDropInventory(Inv);
		}
	}
}

bool ASolCharacter::ServerDropInventory_Validate(AInventoryItem* Inv)
{
	return true;
}

void ASolCharacter::ServerDropInventory_Implementation(AInventoryItem* Inv)
{
	DropInventory(Inv);
}

void ASolCharacter::SetAiming(bool bNewAiming)
{
	bIsAiming = bNewAiming;
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetAiming(bNewAiming);
	}
}

bool ASolCharacter::ServerSetAiming_Validate(bool bNewAiming)
{
	return true;
}

void ASolCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
	SetAiming(bNewAiming);
}

void ASolCharacter::SetSprinting(bool bNewSprinting)
{
	bool OldSprinting = bIsSprinting;
	/** Remove one stamina if new sprint to prevent throttling. **/
	if (OldSprinting == false && bNewSprinting == true)
	{
		Stamina -= 1.f;
	}
	bIsSprinting = bNewSprinting;
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSetSprinting(bNewSprinting);
	}
}

bool ASolCharacter::ServerSetSprinting_Validate(bool bNewSprinting)
{
	return true;
}

void ASolCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}

AActor* ASolCharacter::GetUsableObject()
{
	AActor* UsableObject = nullptr;
	// Perform a trace to get objects.
	// Start vector is just the character's eye location.
	FVector StartVector = GetActorLocation() + FVector(0.f, 0.f, BaseEyeHeight);
	// End vector is 1m in the direction the player is looking, with up to an extra 50cm if the player is looking down (since we cannot crouch yet).
	FVector EndVector = GetControlRotation().Vector();
		float Scalar = 100.f;
	if (EndVector.Z < 0)
	{
		Scalar += (-50.f * EndVector.Z);
	}
	EndVector *= Scalar;
	EndVector += StartVector;

	float TraceRadius = 50.f;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<FHitResult> OutHits;

	bool Result = UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), StartVector, EndVector, TraceRadius, ObjectTypes, true, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true);
	if (Result)
	{
		for (int32 i = 0; i < OutHits.Num(); i++)
		{
			AActor* FoundObject = nullptr;
			UPrimitiveComponent* FoundComp = Cast<UPrimitiveComponent>(OutHits[i].GetComponent());
			if (FoundComp)
			{
				FoundObject = FoundComp->GetOwner();
			}
			APickup* PickupObject = Cast<APickup>(FoundObject);
			// Temporarily forcing this to only consider root components.
			if (PickupObject && PickupObject->GetRootComponent() == FoundComp)
			{
				if ( CanPickupItem(PickupObject->GetHeldItem()) )
				{
					UsableObject = PickupObject;
					// Immediately return this pickup for now.
					return UsableObject;
				}
			}
		}
	}
	return UsableObject;
}

bool ASolCharacter::HandleUse()
{
	// If we're a client, notify the server.
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerHandleUse();
	}

	bool bUseSuccessful = false;
	AActor* UsableObject = GetUsableObject();
	if (UsableObject != nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("Usable Item")));
		APickup* PickupObject = Cast<APickup>(UsableObject);
		if (PickupObject)
		{	
			PickupObject->PickupOnUse(this);
			bUseSuccessful = true;
			if (PickupItemSound && IsLocallyControlled())
			{
				UGameplayStatics::PlaySoundAtLocation(this, PickupItemSound, GetActorLocation());
			}
		}
	}
	if (!bUseSuccessful && UseDenialSound && IsLocallyControlled())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Black, FString::Printf(TEXT("No item!")));
		UGameplayStatics::PlaySoundAtLocation(this, UseDenialSound, GetActorLocation());
	}
	return bUseSuccessful;
}

bool ASolCharacter::ServerHandleUse_Validate()
{
	return true;
}

void ASolCharacter::ServerHandleUse_Implementation()
{
	HandleUse();
}

//////////////////////////////////////////////////////////////////////////
// Weapons and Inventory

bool ASolCharacter::IsFirstPerson() const
{
	return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

USkeletalMeshComponent* ASolCharacter::GetPawnMesh() const
{
	return IsFirstPerson() ? Mesh1P : GetMesh();
}

/** Mesh Functions **/
USkeletalMeshComponent* ASolCharacter::GetSpecificPawnMesh(bool WantFirstPerson) const
{
	return WantFirstPerson == true ? Mesh1P : GetMesh();
}

float ASolCharacter::GetHealth() const
{
	return Health;
}

float ASolCharacter::GetMaxHealth() const
{
	return MaxHealth;
}

float ASolCharacter::GetCappedHealth() const
{
	return MaxHealth - SevereDamage;
}

AWeapon* ASolCharacter::GetEquippedWeapon() const
{
	return EquippedWeapon;
}

AWeapon* ASolCharacter::GetPendingWeapon()
{
	return PendingWeapon;
}

FName ASolCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

FVector ASolCharacter::GetHeadLocation() const
{
	return GetMesh()->GetBoneLocation("head");
}

FVector ASolCharacter::GetTorsoLocation() const
{
	/* Basically centre mass. */
	return GetMesh()->GetBoneLocation("spine_02");
}

void ASolCharacter::TurnOff()
{
	SetWeaponFiringAllowed(false);
	Super::TurnOff();
}

void ASolCharacter::SetWeaponFiringAllowed(bool bInWeaponFiringAllowed)
{
	if (bWeaponFiringAllowed != bInWeaponFiringAllowed)
	{
		bWeaponFiringAllowed = bInWeaponFiringAllowed;
		if (!bWeaponFiringAllowed)
		{
			StopFire();
		}
	}
}

/** Just for debug purposes right now. **/
AWeapon* ASolCharacter::GetSpecificWeapon(int32 WeapNum)
{
	AWeapon* TestWeapon = WeaponInventory[WeapNum];
	if (TestWeapon != NULL)
	{
		return TestWeapon;
	}
	else
	{
		return nullptr;
	}
}

void ASolCharacter::EquipSpecificWeapon(int32 WeapNum)
{
	if (GetInventoryCount() >= (WeapNum + 1) && (EquippedWeapon == NULL || EquippedWeapon->GetWeaponState() != "Equipping"))
	{
		AWeapon* NewWeapon = WeaponInventory[WeapNum];
		/** Don't re-equip the same weapon. **/
		if (NewWeapon && NewWeapon != EquippedWeapon)
		{
			SetPendingWeapon(NewWeapon); // EquipWeapon(NewWeapon);
		}
	}
}

void ASolCharacter::SetEquippedWeapon(class AWeapon* NewWeapon, class AWeapon* LastWeapon)
{
	AWeapon* LocalLastWeapon = NULL;

	if (LastWeapon != NULL)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != EquippedWeapon)
	{
		LocalLastWeapon = EquippedWeapon;
	}

	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnequip();
	}
	EquippedWeapon = NewWeapon;
	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);
		NewWeapon->OnEquip();
	}
}

void ASolCharacter::SetPendingWeapon(class AWeapon* NewWeapon)
{
	PendingWeapon = NewWeapon;

	if (EquippedWeapon != NULL)
	{
		EquippedWeapon->OnUnequip();
	}

	if (EquippedWeapon == NULL)
	{
		EquipWeapon(PendingWeapon);
		PendingWeapon = NULL;
	}
}

void ASolCharacter::OnWeaponUnequipFinish(class AWeapon* OldWeapon)
{
	if (EquippedWeapon == OldWeapon)
	{
		EquippedWeapon = NULL;
	}
	if (PendingWeapon)
	{
		EquipWeapon(PendingWeapon);
		PendingWeapon = NULL;
	}
}

void ASolCharacter::SpawnInitialInventory(TArray<TSubclassOf<AInventoryItem>> DefaultInventoryToAdd, bool bUsePawnDefaultInventory)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}
	int32 NumWeaponClasses = DefaultWeaponClasses.Num();
	if (bUsePawnDefaultInventory)
	{
		for (int32 i = 0; i < NumWeaponClasses; i++)
		{
			if (DefaultWeaponClasses[i])
			{
				CreateNewInventoryItem(DefaultWeaponClasses[i]);
			}
		}
	}
	NumWeaponClasses = DefaultInventoryToAdd.Num();
	if (0 < NumWeaponClasses)
	{
		for (int32 i = 0; i < NumWeaponClasses; i++)
		{
			if (DefaultInventoryToAdd[i])
			{
				CreateNewInventoryItem(DefaultInventoryToAdd[i]);
			}
		}
	}

	// Equip the best weapon
	if (!EquippedWeapon)
	{
		if (PrimaryWeapon)
		{
			EquipWeapon(PrimaryWeapon);
		}
		else if (SecondaryWeapon)
		{
			EquipWeapon(SecondaryWeapon);
		}
		else if (SidearmWeapon)
		{
			EquipWeapon(SidearmWeapon);
		}
		else
		{
			for (int32 i = 0; i < Equipment.Num(); i++)
			{
				if (Equipment[i])
				{
					EquipWeapon(Equipment[i]);
				}
			}
		}
	}
}

int32 ASolCharacter::GetInventoryCount() const
{
	return WeaponInventory.Num();
}

void ASolCharacter::DestroyInventory()
{
	for (int32 i = 0; i < GetInventoryCount(); i++)
	{
		if (WeaponInventory[i]->GetOwningPawn() != this && WeaponInventory[i]->GetOwningPawn() != nullptr)
		{
			UE_LOG(LogDamage, Warning, TEXT("%s: Item %s belonged to another pawn!"), *GetName(), *ItemInventory[i]->GetName());
		}
		WeaponInventory[i]->SetOwningPawn(NULL);
		WeaponInventory[i]->Destroy();
	}
	
	for (int32 i = 0; i < ItemInventory.Num(); i++)
	{
		UE_LOG(LogDamage, Warning, TEXT("%s: Deleting Item %s."), *GetName(), *ItemInventory[i]->GetName());
		if (ItemInventory[i]->GetOwningPawn() != this && ItemInventory[i]->GetOwningPawn() != nullptr)
		{
			UE_LOG(LogDamage, Warning, TEXT("%s: Item %s belonged to another pawn!"), *GetName(), *ItemInventory[i]->GetName());
		}
		ItemInventory[i]->SetOwningPawn(NULL);
		ItemInventory[i]->Destroy();
	}
}

AInventoryItem* ASolCharacter::CreateNewInventoryItem(TSubclassOf<class AInventoryItem> NewItemClass)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return nullptr;
	}
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AInventoryItem* NewItem = GetWorld()->SpawnActor<AInventoryItem>(NewItemClass, SpawnInfo);
	AddToInventory(NewItem);
	return NewItem;
}


void ASolCharacter::AddToInventory(AInventoryItem* NewItem)
{
	if (NewItem != nullptr && GetLocalRole() == ROLE_Authority)
	{
		ItemInventory.Add(NewItem);
		AWeapon* NewWeapon = Cast<AWeapon>(NewItem);
		if (NewWeapon)
		{
			FName NewWeaponSlot = NewWeapon->WeaponSlotType;
			//AFirearm* NewFirearm = Cast<AFirearm>(NewWeapon);
			/* Add the weapon to the right inventory slot. */
			if (NewWeaponSlot == WeaponSlotType::Main)
			{
				if (PrimaryWeapon == NULL)
				{
					PrimaryWeapon = NewWeapon;
				}
				else if (PrimaryWeapon->WeaponSlotType == WeaponSlotType::Sidearm && SidearmWeapon == NULL)
				{
					SidearmWeapon = PrimaryWeapon;
					PrimaryWeapon = NewWeapon;
				}
				else if (SecondaryWeapon == NULL)
				{
					SecondaryWeapon = NewWeapon;
				}
				else if (SecondaryWeapon->WeaponSlotType == WeaponSlotType::Sidearm && SidearmWeapon == NULL)
				{
					SidearmWeapon = SecondaryWeapon;
					SecondaryWeapon = NewWeapon;
				}
			}
			else if (NewWeaponSlot == WeaponSlotType::Sidearm)
			{
				if (SidearmWeapon == NULL)
				{
					SidearmWeapon = NewWeapon;
				}
				else if (PrimaryWeapon == NULL)
				{
					PrimaryWeapon = NewWeapon;
				}
				else if (SecondaryWeapon == NULL)
				{
					SecondaryWeapon = NewWeapon;
				}
			}
			else if (NewWeaponSlot == WeaponSlotType::Equipment && Equipment.Num() < MaxEquipment)
			{
				Equipment.AddUnique(NewWeapon);
			}

			WeaponInventory.Remove(NewWeapon);
			RemakeWeaponList();
		}
		NewItem->OnEnterInventory(this);
	}
}

void ASolCharacter::RemoveFromInventory(AInventoryItem* ItemToRemove)
{
	if (ItemToRemove && GetLocalRole() == ROLE_Authority)
	{
		ItemInventory.Remove(ItemToRemove);
		AWeapon* RemovedWeapon = Cast<AWeapon>(ItemToRemove);
		if (RemovedWeapon)
		{
			if (RemovedWeapon == EquippedWeapon)
			{
				EquippedWeapon = nullptr;
			}
			if (RemovedWeapon == PendingWeapon)
			{
				PendingWeapon = nullptr;
			}
			if (RemovedWeapon == PrimaryWeapon)
			{
				PrimaryWeapon = nullptr;
			}
			if (RemovedWeapon == SecondaryWeapon)
			{
				SecondaryWeapon = nullptr;
			}
			if (RemovedWeapon == SidearmWeapon)
			{
				SidearmWeapon = nullptr;
			}
			Equipment.Remove(RemovedWeapon);

			WeaponInventory.Remove(RemovedWeapon);
			RemakeWeaponList();
		}
		ItemToRemove->OnLeaveInventory();
	}
}

void ASolCharacter::RemakeWeaponList()
{
	/* Rearrange the inventory list. */
	TArray<AWeapon*> OldWeaponInv = WeaponInventory;
	TArray<AWeapon*> NewWeaponInv;
	if (SidearmWeapon != NULL)
	{
		NewWeaponInv.AddUnique(SidearmWeapon);
		OldWeaponInv.Remove(SidearmWeapon);
	}
	if (PrimaryWeapon != NULL)
	{
		NewWeaponInv.AddUnique(PrimaryWeapon);
		OldWeaponInv.Remove(PrimaryWeapon);
	}
	if (SecondaryWeapon != NULL)
	{
		NewWeaponInv.AddUnique(SecondaryWeapon);
		OldWeaponInv.Remove(SecondaryWeapon);
	}
	for (int32 i = 0; i < Equipment.Num(); i++)
	{
		if (Equipment[i] != NULL)
		{
			NewWeaponInv.AddUnique(Equipment[i]);
			OldWeaponInv.Remove(Equipment[i]);
		}
	}
	/* For now, append anything that doesn't fit in our slots to the end. */
	for (int32 i = 0; i < OldWeaponInv.Num(); i++)
	{
		if (OldWeaponInv[i] != NULL)
		{
			NewWeaponInv.Add(OldWeaponInv[i]);
		}
	}
	WeaponInventory = NewWeaponInv;
}

bool ASolCharacter::CanPickupItem(AInventoryItem* Item) const
{
	AFirearm* TestFirearm = Cast<AFirearm>(Item);
	if (TestFirearm)
	{
		// Currently, limit the player to eight inventory items.
		if (GetInventoryCount() >= 8)
		{
			return false;
		}
		FName SlotType = TestFirearm->WeaponSlotType;
		////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("InItemType: %s."), *SlotType.ToString()));
		if (SlotType == WeaponSlotType::Main && PrimaryWeapon != NULL && SecondaryWeapon != NULL) //(SlotType == WeaponSlotType::Sidearm || SlotType == WeaponSlotType::Main)
		{
			return false;
		}
		if (SlotType == WeaponSlotType::Sidearm && SidearmWeapon != NULL && PrimaryWeapon != NULL && SecondaryWeapon != NULL)
		{
			return false;
		}
		if (SlotType == WeaponSlotType::Equipment && Equipment.Num() >= MaxEquipment)
		{
			return false;
		}
	}
	return true;
}

AInventoryItem* ASolCharacter::CanSwapForItem(AInventoryItem* Item) const
{
	AFirearm* TestFirearm = Cast<AFirearm>(Item);
	if (TestFirearm)
	{
		FName ItemSlotType = TestFirearm->WeaponSlotType;
		if (EquippedWeapon)
		{
			if (EquippedWeapon->WeaponSlotType == ItemSlotType)
			{
				return EquippedWeapon;
			}
		}
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Weapon Location/Rotation

/** Here's the current system:
    + OnCameraUpdate() positions the weapon's 1P mesh.
	+ GetWeaponRotationOffset() and GetWeaponLocationOffset() calculate the offset from the camera's
	  origin rotation and location.
	+ GetWeaponAimRot() and GetWeaponAimLoc() find the rot/loc of where the weapon's "muzzle" is. It
	  combines the player camera's location/rotation with the previously mentioned GetWeaponRotationOffset() 
	  and GetWeaponLocationOffset() for projectile spawning. Note that shot spread/inaccuracy is handled at
	  the weapon level (e.g. GetAdjustedAimRot()).
	+ Currently, UpdateWeapnRotationAndLocation() only uses GetWeaponLocationOffset(). Rotation offset is solely
	  used to spawn projectiles; once we get a proper "origin" working, we can reflect the offset in the mesh.
	+ TODO: 
	  + The gun's origin rot/loc and the muzzle rot/loc should probably be calculated separately. E.g:
	    GetWeaponAimRot() and GetWeaponRotation() functions. The latter function can then be used for both
		GetWeaponAimRot() and OnCameraUpdate().
	  + Figure out how to correctly offset the weapon mesh's rotation.
	+ AddRecoil() is called by the weapon; it adds recoil to the player's recoil aim offset.
	+ ProcessRecoil() is called every tick if there is recoil. It adjusts the offset over time.
	*/

FRotator ASolCharacter::AddWeaponOffset(FRotator RotationToAdd, float MaxPitch, float MaxYaw)
{
	// These are the actual pitch and yaw offset values we can add.
	// TODO: Clamp values within a circle/oval instead of a square/rectangle.
	const float ClampedPitch = FMath::Clamp(RotationToAdd.Pitch, 
											FMath::Min(-MaxPitch - HeldWeaponOffset.Pitch, 0.0f),
											FMath::Max(MaxPitch - HeldWeaponOffset.Pitch, 0.0f));
	const float ClampedYaw = FMath::Clamp(RotationToAdd.Yaw,
										  FMath::Min(-MaxYaw - HeldWeaponOffset.Yaw, 0.0f),
										  FMath::Max(MaxYaw - HeldWeaponOffset.Yaw, 0.0f));
	// Add pitch/yaw offset.
	HeldWeaponOffset += FRotator(ClampedPitch, ClampedYaw, 0.0f);
	// Calculate how much pitch/yaw we were unable to add.
	const float PitchRemainder = RotationToAdd.Pitch - ClampedPitch;
	const float YawRemainder = RotationToAdd.Yaw - ClampedYaw;
	// Return the remainder in case it's wanted (e.g. camera recoil).
	////GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, FString::Printf(TEXT("In: %f %f  Clamped: %f %f  Remainder: %f %f  Max: %f %f"), RotationToAdd.Pitch, RotationToAdd.Yaw, ClampedPitch, ClampedYaw, PitchRemainder, YawRemainder, MaxPitch, MaxYaw));
	return FRotator(PitchRemainder, YawRemainder, 0.0f);
}

void ASolCharacter::FaceRotation(FRotator NewControlRotation, float DeltaTime)
{
	FRotator NCR = NewControlRotation;
	FRotator LCR = LastControlRotation;

	float DeltaAimLagPitch = (NCR.Pitch - LCR.Pitch);
	if (FMath::Abs(DeltaAimLagPitch) > FMath::Abs(360.f + NCR.Pitch - LCR.Pitch))
	{
		DeltaAimLagPitch = (360.f + NCR.Pitch - LCR.Pitch);
	}
	if (FMath::Abs(DeltaAimLagPitch) > FMath::Abs(-360.f + NCR.Pitch - LCR.Pitch))
	{
		DeltaAimLagPitch = (-360.f + NCR.Pitch - LCR.Pitch);
	}
	float DeltaAimLagYaw = (NCR.Yaw - LCR.Yaw);
	if (FMath::Abs(DeltaAimLagYaw) > FMath::Abs(360.f + NCR.Yaw - LCR.Yaw))
	{
		DeltaAimLagYaw = (360.f + NCR.Yaw - LCR.Yaw);
	}
	if (FMath::Abs(DeltaAimLagYaw) > FMath::Abs(-360.f + NCR.Yaw - LCR.Yaw))
	{
		DeltaAimLagYaw = (-360.f + NCR.Yaw - LCR.Yaw);
	}

	bool bThreshhold = false;
	while (DeltaAimLagPitch > 360.f || DeltaAimLagPitch < -360.f)
	{
		if (DeltaAimLagPitch > 360.f)
		{
			DeltaAimLagPitch -= 360.f;
		}
		else if (DeltaAimLagPitch < -360.f)
		{
			DeltaAimLagPitch += 360.f;
		}
		bThreshhold = true;
	}
	while (DeltaAimLagYaw > 360.f || DeltaAimLagYaw < -360.f)
	{
		if (DeltaAimLagYaw > 360.f)
		{
			DeltaAimLagYaw -= 360.f;
		}
		else if (DeltaAimLagYaw < -360.f)
		{
			DeltaAimLagYaw += 360.f;
		}
		bThreshhold = true;
	}
	if (bThreshhold)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, FString::Printf(TEXT("THRESHHOLD! THE END IS NIGH!")));
	}
	if (DeltaAimLagPitch != 0 || DeltaAimLagYaw != 0)
	{
		const float AimLagScalar = 0.5f;
		AddWeaponOffset(FRotator(DeltaAimLagPitch * AimLagScalar, DeltaAimLagYaw * AimLagScalar, 0.0f), MaxFreeAimRadius, MaxFreeAimRadius);
	}

	LastControlRotation = NewControlRotation;
	
	Super::FaceRotation(NewControlRotation, DeltaTime);
}

void ASolCharacter::OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	// Move to camera class in the future.
	const float DefaultFOV = 90.f;
	const float AimedFOV = 75.f;
	FirstPersonCameraComponent->FieldOfView = FMath::LerpStable(DefaultFOV, AimedFOV, CurrentAimPct);
	// END

	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("CharacterMesh1P")));

	FVector MeshLoc = DefMesh1P->GetRelativeLocation();
	float EyeHeightTemp = bIsCrouched ? CrouchedEyeHeight : BaseEyeHeight;
	FVector EyeHeightVector = FVector(0.0f, 0.0f, EyeHeightTemp);
	FirstPersonCameraComponent->SetRelativeLocation(EyeHeightVector);
	FVector TempOffset = FVector(-27.5f, 0.5f, -35.f) + EyeHeightVector; //FVector(-25.f, 1.f, -37.5f);  //23.49, 0, 141.24 vs 0, 0, 165
	FVector TestLocFinal = MeshLoc + TempOffset + GetWeaponLocationOffset();

	const FMatrix DefMeshLS = FRotationTranslationMatrix(DefMesh1P->GetRelativeRotation(), TestLocFinal); // DefMesh1P->RelativeLocation
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch(CameraRotation.Pitch, 0.0f, 0.0f);
	const FRotator RotCameraYaw(0.0f, CameraRotation.Yaw, 0.0f);

	const FRotator RotationOffset(GetWeaponRotationOffset().Pitch, GetWeaponRotationOffset().Yaw, 0.0f); // TEST

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix(RotCameraYaw, CameraLocation) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix(RotCameraPitch) * LeveledCameraLS;
	const FMatrix OffsetPitchedCameraLS = FRotationMatrix(RotationOffset) * PitchedCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * OffsetPitchedCameraLS;

	Mesh1P->SetRelativeLocationAndRotation(PitchedMesh.GetOrigin(), PitchedMesh.Rotator());
}

FRotator ASolCharacter::GetWeaponRotationOffset() const
{
	// The clamp probably isn't necessary, but I like to be safe.
	const float AimScalar = FMath::Clamp(1.0f - (CurrentAimPct * 0.8f), 0.0f, 1.0f);
	const FRotator FinalRot = HeldWeaponOffset * AimScalar;
	return FinalRot;
}

FVector ASolCharacter::GetWeaponLocationOffset() const
{
	const AWeapon* Weapon = GetEquippedWeapon();
	FVector AimedVector = FVector::ZeroVector;
	FVector UnaimedVector = FVector::ZeroVector;
	if (Weapon)
	{
		UnaimedVector += Weapon->GetWeaponOffset();
		AFirearm* Firearm = Cast<AFirearm>(EquippedWeapon);
		if (Firearm)
		{
			AimedVector += Firearm->GetAimedOffset();
		}
	}
	// Function to make interpolation sigmoid (i.e. more natural).
	const float ModifiedAimPct = 0.5f + ((2 * (CurrentAimPct - 0.5f)) / (1 + (2 * FMath::Abs(CurrentAimPct - 0.5f))));
	const FVector FinalVector = FMath::LerpStable(UnaimedVector, AimedVector, ModifiedAimPct);
	return FinalVector;
}

FRotator ASolCharacter::GetWeaponAimRot() const
{
	// NOTE: GetBaseAimRotation().Clamp() = GetControlRotation()
	const FRotator FinalRotation = GetBaseAimRotation().Clamp() + GetWeaponRotationOffset().Clamp();
	return FinalRotation; 
}

FVector ASolCharacter::GetWeaponAimLoc() const
{
	const FVector EyeLocation = GetActorLocation() + FVector(0.f, 0.f, BaseEyeHeight);
	const FVector WeaponOffset = GetWeaponLocationOffset() + FVector(75.f, 0.f, -7.5f); //Temporary offset for muzzle.
	const FVector FinalLocation = EyeLocation + GetWeaponAimRot().RotateVector(WeaponOffset);
	return FinalLocation;
}

void ASolCharacter::AddRecoil(FVector InRecoil)
{
	if (RecoilCurveTime >= 0.0f)
	{
		CurrentRecoilVelocity = FVector::ZeroVector;
		LastRecoilVelocity = FVector::ZeroVector;
	}
	CurrentRecoilVelocity += (InRecoil * 0.1f);
	LastRecoilVelocity = CurrentRecoilVelocity;
	RecoilCurveTime = 1.0f;
	/* Currently a default property, but we could make it a weapon-specific input. */
	//RecoilTimeScalar = 0.5f;
}

void ASolCharacter::ProcessRecoil(float DeltaSeconds)
{
	if (RecoilCurveTime > 0.0f)
	{
		// Make a pseudo-curve (faster to slower).
		const float ScaledTime = DeltaSeconds / RecoilTimeScalar;
		const float RecoilThisTick = FMath::Clamp(ScaledTime, 0.0f, RecoilCurveTime);
		const float RecoilCurveStart = FMath::Max(0.0f, 2 * RecoilCurveTime);
		const float RecoilCurveEnd = FMath::Max(0.0f, 2 * (RecoilCurveTime - RecoilThisTick));

		// Calculate recoil to apply this tick.
		const float RecoilCurveAverage = (RecoilCurveStart + RecoilCurveEnd) / 2;
		const FVector TotalRecoilThisTick = RecoilThisTick * RecoilCurveAverage * LastRecoilVelocity;

		/* In the future, we will apply recoil to both the weapon's offset and the player's view.
			How much goes to the view  vs. weapon offset largely depends on if the weapon is aimed.
			However, we'll have to add view recoil by a method other than AddControllerPitch/YawInput() for best results.
			So, we'll just add it to the weapon offset for now. */
		//const float PctCameraRecoil = FMath::Clamp(CurrentAimPct, 0.1f, 0.9f);
		//const FVector CameraRecoilThisTick = TotalRecoilThisTick * PctCameraRecoil;
		//const FVector OffsetRecoilThisTick = TotalRecoilThisTick - CameraRecoilThisTick;
		AddWeaponOffset(FRotator(TotalRecoilThisTick.X, TotalRecoilThisTick.Y, 0.0f), MaxRecoilOffsetRadius, MaxRecoilOffsetRadius); // REMOVE ME IF UNCOMMENTING OTHER CODE.
		//FRotator Remainder = AddWeaponOffset(FRotator(OffsetRecoilThisTick.X, OffsetRecoilThisTick.Y, 0.0f), 15.f, 15.f);
		//AddControllerPitchInput(-CameraRecoilThisTick.X - Remainder.Pitch);
		//AddControllerYawInput(-CameraRecoilThisTick.Y - Remainder.Yaw);

		// Finally, update the recoil.
		const float NewRecoilTime = FMath::Max(RecoilCurveTime - RecoilThisTick, 0.0f);
		RecoilCurveTime = NewRecoilTime;
		CurrentRecoilVelocity -= TotalRecoilThisTick;
	}
}

//////////////////////////////////////////////////////////////////////////
// Movement

void ASolCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASolCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ASolCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASolCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//////////////////////////////////////////////////////////////////////////
// Death and Dying

float ASolCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Health <= 0.f)
	{
		return 0.f;
	}

	// Modify based on game rules.
	ASolGameMode* const Game = GetWorld()->GetAuthGameMode<ASolGameMode>();
	Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		//(const FPointDamageEvent&)DamageEvent
		FHitResult HitInfo;
		FVector Dir;
		// The API reference for FDamageEvent says this will "ideally" go away after time.
		// That said, this is so far the only way I can figure out how to retrieve the HitInfo, since "casting" doesn't work.
		DamageEvent.GetBestHitInfo(this, DamageCauser, HitInfo, Dir); //(const FPointDamageEvent&)DamageEvent.GetBestHitInfo.BoneName;
		FName HitBone = HitInfo.BoneName;

		Damage = LocalizeDamage(Damage, HitBone);
	}

	const float DamageToHealth = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (DamageToHealth > 0.f)
	{
		Health -= DamageToHealth;
		SevereDamage += DamageToHealth * SevereDamageScalar;

		if (Health <= 0)
		{
			
			Die(DamageToHealth, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(DamageToHealth, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
			// Start health regen if we are not doing so already.
			if (Health < MaxHealth && !GetWorldTimerManager().IsTimerActive(TimerHandle_RegenHealth))
			{
				GetWorldTimerManager().SetTimer(TimerHandle_RegenHealth, this, &ASolCharacter::RegenHealth, 1 / HealthTicksPerSec, true);
			}
		}
		//MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}
	return Damage;
}

float ASolCharacter::LocalizeDamage(float Damage, FName HitBone)
{
	// TODO: Make arrays of bone names for each body part.
	// TODO: Add events other than damage scaling.
	float DamagePctAtZero = 1.f;
	float DamagePctAtEnd = 1.f;
	float DamageCurveEnd = GetMaxHealth();
	// Head and neck
	if (HitBone == "neck_01" || HitBone == "head")
	{
		DamagePctAtEnd = 4.f;
		DamageCurveEnd = 25.f;
	}
	// Shoulders? (clavicle)
	else if (HitBone == "clavicle_l" || HitBone == "clavicle_r")
	{
		DamagePctAtZero = 0.875f;
		DamagePctAtEnd = 0.75f;
	}
	// Upper body
	else if (HitBone == "spine_03" || HitBone == "spine_02")
	{
		// No damage scaling
	}
	// Pelvis && stomach
	else if (HitBone == "pelvis" || HitBone == "spine_01")
	{
		DamagePctAtZero = 1.f;
		DamagePctAtEnd = 0.875f;
	}
	// Legs
	else if (HitBone == "thigh_l" || HitBone == "calf_l" || HitBone == "foot_l" || HitBone == "ball_l"
		|| HitBone == "thigh_r" || HitBone == "calf_r" || HitBone == "foot_r" || HitBone == "ball_r")
	{
		DamagePctAtZero = 0.875f;
		DamagePctAtEnd = 0.625f;
	}
	// Arms (do last since there are so many parts to check)
	//      (but, currently, identical to being hit in the legs)
	else if (HitBone == "upperarm_l" || HitBone == "lowerarm_l" || HitBone == "hand_l"
		|| HitBone == "upperarm_r" || HitBone == "lowerarm_r" || HitBone == "hand_r"
		|| HitBone == "index_01_l" || HitBone == "index_02_l" || HitBone == "index_03_l"
		|| HitBone == "index_01_r" || HitBone == "index_02_r" || HitBone == "index_03_r"
		|| HitBone == "middle_01_l" || HitBone == "middle_02_l" || HitBone == "middle_03_l"
		|| HitBone == "middle_01_r" || HitBone == "middle_02_r" || HitBone == "middle_03_r"
		|| HitBone == "ring_01_l" || HitBone == "ring_02_l" || HitBone == "ring_03_l"
		|| HitBone == "ring_01_r" || HitBone == "ring_02_r" || HitBone == "ring_03_r"
		|| HitBone == "pinky_01_l" || HitBone == "pinky_02_l" || HitBone == "pinky_03_l"
		|| HitBone == "pinky_01_r" || HitBone == "pinky_02_r" || HitBone == "pinky_03_r"
		|| HitBone == "thumb_01_l" || HitBone == "thumb_02_l" || HitBone == "thumb_03_l"
		|| HitBone == "thumb_01_r" || HitBone == "thumb_02_r" || HitBone == "thumb_03_r")
	{
		DamagePctAtZero = 0.875f;
		DamagePctAtEnd = 0.625f;
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("TakeDamage(): Where was %s hit?!"), *this->GetName()));
	}
	const float DamageScalar = FMath::Lerp(DamagePctAtZero, DamagePctAtEnd, FMath::Clamp(Damage / DamageCurveEnd, 0.f, 1.f));
	const float FinalDamage = Damage * DamageScalar;
	// Fun debug stuff
	/*if (GetPlayerState())
	{
		float RandFloat = FMath::FRand();
		RandFloat *= RandFloat;
		if (FMath::RandBool())
		{
			RandFloat *= -1;
		}
		float RandTest = Damage + (Damage * 0.1 * RandFloat);
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s hit in %s: %f became %f (%f). Could have been %f."), 
			*this->GetPlayerState()->GetPlayerName(),
			*HitBone.ToString(),
			Damage, 
			FinalDamage, 
			FinalDamage / Damage * 100,
			RandTest));
	}*/
	return FinalDamage;
}

bool ASolCharacter::IsAlive() const
{
	return Health > 0 && !bIsDying;
}

bool ASolCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| GetLocalRole() != ROLE_Authority)			// not authority
	{
		return false;
	}
	if (GetWorld()->GetAuthGameMode() == NULL)
	{
		return false;
	}
	else
	{
		AGameMode* GameMode = Cast<AGameMode>(GetWorld()->GetAuthGameMode());
		if (GameMode == NULL || GameMode->GetMatchState() == MatchState::LeavingMap)
		{
			return false; // level transition occurring
		}
	}
	return true;
}


bool ASolCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0.0f, Health);

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<ASolGameMode>()->OnPlayerDeath(Killer, KilledPlayer, this, DamageType);

	NetUpdateFrequency = GetDefault<ASolCharacter>()->NetUpdateFrequency;
	GetCharacterMovement()->ForceReplicationUpdate();

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}


void ASolCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	SetReplicatingMovement(false);
	TearOff(); //bTearOff = true;
	bIsDying = true;

	if (GetLocalRole() == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

		// play the force feedback effect on the client player controller
		/**
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
		UShooterDamageType *DamageType = Cast<UShooterDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
		if (DamageType && DamageType->KilledForceFeedback)
		{
		PC->ClientPlayForceFeedback(DamageType->KilledForceFeedback, false, "Damage");
		}
		}
		**/
	}
	// cannot use IsLocallyControlled here, because even local client's controller may be NULL here
	if (GetNetMode() != NM_DedicatedServer && DyingSound && Mesh1P && Mesh1P->IsVisible())
	{
		UGameplayStatics::PlaySoundAtLocation(this, DyingSound, GetActorLocation());
	}

	// Later relegate to another function, such as "DestroyInventory()"? **/
	if (GetLocalRole() == ROLE_Authority)
	{
		if (EquippedWeapon)
		{
			DropInventory(EquippedWeapon);
		}
		while (ItemInventory.IsValidIndex(0))
		{
			DropInventory(ItemInventory[0]);
		}

		// Drop up to three bags of ammo loot.
		/*for (int32 i = 0; i < 2; i++)
		{
			AAmmoBag* LootBag = Cast<AAmmoBag>(CreateNewInventoryItem(AAmmoBag::StaticClass()));
			for (int32 j = 0; j < ItemInventory.Num(); j++)
			{
				AAmmo* AmmoItem = Cast<AAmmo>(ItemInventory[j]);
				if (AmmoItem)
				{
					if (LootBag->ContainedItems.Num() == 0 || LootBag->ContainedItems[0]->GetClass() == AmmoItem->GetClass())
					{
						LootBag->ContainedItems.Add(AmmoItem);
						RemoveFromInventory(AmmoItem);
						UE_LOG(LogDamage, Warning, TEXT("%s: Removed item %s from inventory."), *GetName(), *AmmoItem->GetName());
					}
				}
			}
			if (LootBag->ContainedItems.Num() <= 1)
			{
				if (LootBag->ContainedItems.Num() == 1)
				{
					DropInventory(LootBag->ContainedItems[0]);
					LootBag->ContainedItems.Empty();
				}
				RemoveFromInventory(LootBag);
				LootBag->Destroy();
			}
			else
			{
				DropInventory(LootBag);
			}
		}*/

		DestroyInventory();
	}

	// switch back to 3rd person view
	//UpdatePawnMeshes();

	DetachFromControllerPendingDestroy();
	//StopAllAnimMontages();
	/**
	if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
	{
	LowHealthWarningPlayer->Stop();
	}

	if (RunLoopAC)
	{
	RunLoopAC->Stop();
	}
	**/
	// disable collisions on capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	SetRagdollPhysics();
}

void ASolCharacter::RegenHealth()
{
	const float CappedHealth = GetCappedHealth();
	if (IsAlive() && Health < CappedHealth)
	{
		const float HealthToRegen = HealthRegenRate / HealthTicksPerSec;
		Health = FMath::Min(CappedHealth, Health + HealthToRegen);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_RegenHealth);
	}
}

void ASolCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	UE_LOG(LogDamage, Warning, TEXT("PlayHit() -> BEFORE : GetLocalRole() == ROLE_Authority"));
	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogDamage, Warning, TEXT("PlayHit() -> GetLocalRole() == ROLE_Authority"));
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, false);
		/**
		// play the force feedback effect on the client player controller
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UShooterDamageType *DamageType = Cast<UShooterDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->HitForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->HitForceFeedback, false, "Damage");
			}
		}
		**/
	}

	/**
	if (DamageTaken > 0.f)
	{
		ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
	}
	**/
	ASolPlayerController* MyPC = Cast<ASolPlayerController>(Controller);
	ASolHUD* MyHUD = MyPC ? Cast<ASolHUD>(MyPC->GetHUD()) : NULL;
	if (MyHUD)
	{
		MyHUD->NotifyPlayerHit(DamageTaken); // , DamageEvent, PawnInstigator);
	}
	/**
	if (PawnInstigator && PawnInstigator != this && PawnInstigator->IsLocallyControlled())
	{
		AShooterPlayerController* InstigatorPC = Cast<AShooterPlayerController>(PawnInstigator->Controller);
		AShooterHUD* InstigatorHUD = InstigatorPC ? Cast<AShooterHUD>(InstigatorPC->GetHUD()) : NULL;
		if (InstigatorHUD)
		{
			InstigatorHUD->NotifyEnemyHit();
		}
	}
	**/
}

void ASolCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		// initialize physics/etc
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!bInRagdoll)
	{
		// hide and set short lifespan
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(60.0f);
	}
}

void ASolCharacter::Landed(const FHitResult& Hit)
{
	if (!bClientUpdating)
	{
		TakeFallingDamage(Hit, GetCharacterMovement()->Velocity.Z);
	}
	Super::Landed(Hit);
}

void ASolCharacter::FellOutOfWorld(const UDamageType& DmgType)
{
	if (IsAlive())
	{
		FDamageEvent DamageEvent;
		DamageEvent.DamageTypeClass = DmgType.GetClass();
		Die(999, DamageEvent, nullptr, nullptr);
	}
	else
	{
		Super::FellOutOfWorld(DmgType);
	}
}

void ASolCharacter::TakeFallingDamage(const FHitResult& Hit, float FallingVelocity)
{
	const float MaxSafeFallingSpeed = 2000.f;
	{
		if (FallingVelocity < -1.f * MaxSafeFallingSpeed)
		{
			const float FallingDamage = -100.f * FMath::Abs(FallingVelocity + MaxSafeFallingSpeed) / MaxSafeFallingSpeed;
			if (FallingDamage > 0.0f)
			{
				// TODO: Make falling damage type.
				FPointDamageEvent DamageEvent(FallingDamage, Hit, GetCharacterMovement()->Velocity.GetSafeNormal(), UDamageType::StaticClass());
				TakeDamage(FallingDamage, DamageEvent, Controller, this);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Visuals / Mesh

void ASolCharacter::UpdateAllMaterials()
{
	for (int32 i = 0; i < MeshMIDs.Num(); i++)
	{
		UpdateMaterialColors(MeshMIDs[i]);
	}
	for (int32 i = 0; i < Mesh1PMIDs.Num(); i++)
	{
		UpdateMaterialColors(Mesh1PMIDs[i]);
	}
}

void ASolCharacter::UpdateMaterialColors(UMaterialInstanceDynamic* UseMID)
{
	if (UseMID)
	{
		UseMID->SetVectorParameterValue(TEXT("Primary Colour"), GetPrimaryColor());
		UseMID->SetVectorParameterValue(TEXT("Secondary Colour"), GetSecondaryColor());
	}
}

FLinearColor ASolCharacter::GetPrimaryColor()
{
	if (PrimaryColorOverride != FLinearColor::Transparent)
	{
		return PrimaryColorOverride;
	}
	
	FLinearColor PrimaryColor(0.25f, 0.25f, 0.35f);
	
	ASolPlayerState* MyPlayerState = Cast<ASolPlayerState>(GetPlayerState());
	if (MyPlayerState)
	{
		PrimaryColor = MyPlayerState->GetPrimaryColor();
		ATeamState* MyTeam = MyPlayerState->GetTeam();
		if (MyTeam)
		{
			PrimaryColor = MyTeam->GetTeamColor();
		}
	}
	return PrimaryColor;
}

FLinearColor ASolCharacter::GetSecondaryColor()
{
	if (SecondaryColorOverride != FLinearColor::Transparent)
	{
		return SecondaryColorOverride;
	}
	
	FLinearColor SecondaryColor(0.75f, 0.0f, 0.75f);

	ASolPlayerState* MyPlayerState = Cast<ASolPlayerState>(GetPlayerState());
	if (MyPlayerState)
	{
		SecondaryColor = MyPlayerState->GetSecondaryColor();
	}
	return SecondaryColor;
}

//////////////////////////////////////////////////////////////////////////
// Animations

float ASolCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}

	return 0.0f;
}

void ASolCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		// AnimMontage->BlendOutTime is no longer a member of UAnimMontage in UE 4.11
		// UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOutTime);
		UseMesh->AnimScriptInstance->Montage_Stop(0.1);
	}
}

void ASolCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ASolCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner: weapon change requests are locally instigated, other clients don't need it
	DOREPLIFETIME_CONDITION(ASolCharacter, WeaponInventory, COND_OwnerOnly);

	// everyone except local owner: flag change is locally instigated
	DOREPLIFETIME_CONDITION(ASolCharacter, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASolCharacter, bIsSprinting, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ASolCharacter, LastTakeHitInfo, COND_Custom);

	// everyone
	DOREPLIFETIME(ASolCharacter, EquippedWeapon);
	DOREPLIFETIME(ASolCharacter, SidearmWeapon);
	DOREPLIFETIME(ASolCharacter, PrimaryWeapon);
	DOREPLIFETIME(ASolCharacter, SecondaryWeapon);
	DOREPLIFETIME(ASolCharacter, Health);
}

bool ASolCharacter::IsAiming() const
{
	return bIsAiming;
}

float ASolCharacter::GetAimPct() const
{
	return CurrentAimPct;
}

bool ASolCharacter::IsSprinting() const
{
	return bIsSprinting;
}

void ASolCharacter::OnRep_EquippedWeapon(class AWeapon* LastWeapon)
{
	SetEquippedWeapon(EquippedWeapon, LastWeapon);
}

/** Taken almost verbatim from ShooterGame example. **/
void ASolCharacter::ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled)
{
	//const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	/**
	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((PawnInstigator == LastTakeHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) && (LastTakeHitTimeTimeout == TimeoutTime))
	{
		// same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		Damage += LastTakeHitInfo.ActualDamage;
	}
	**/

	LastTakeHitInfo.ActualDamage = Damage;
	LastTakeHitInfo.PawnInstigator = Cast<ASolCharacter>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();

	//LastTakeHitTimeTimeout = TimeoutTime;
}

void ASolCharacter::PossessedBy(class AController* InController)
{
	Super::PossessedBy(InController);

	// [server] as soon as PlayerState is assigned, set team colors of this pawn for local player
	UpdateAllMaterials();
}

void ASolCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// [client] as soon as PlayerState is assigned, set team colors of this pawn for local player
	if (GetPlayerState() != NULL)
	{
		UpdateAllMaterials();
	}
}

void ASolCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
}