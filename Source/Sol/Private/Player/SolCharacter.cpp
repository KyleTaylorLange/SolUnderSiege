;// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "SolCharacter.h"
#include "Sol.h"
#include "UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h" // For the sphere trace.
#include "SolGameMode.h"
#include "SolPlayerState.h"
#include "TeamState.h"
#include "SolGameState.h"
#include "InventoryItem.h"
#include "SolDamageType.h"
#include "Firearm.h"
#include "Ammo.h"
#include "AmmoBag.h"
#include "SolPlayerController.h"
#include "SolHUD.h"
#include "InteractionEvent.h"
#include "InteractableComponent.h"
#include "InventoryComponent.h"
#include "Pickup.h"
#include "SolCharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

//////////////////////////////////////////////////////////////////////////
// ASolCharacter

ASolCharacter::ASolCharacter(const FObjectInitializer& ObjectInitializer) //: Super(ObjectInitializer)
  : Super(ObjectInitializer.SetDefaultSubobjectClass<USolCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(47.5f, 87.5f); //175cm tall, 112.5cm crouching (56.25cm HH)
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->CanCharacterStepUpOn = ECB_Yes;
	BaseEyeHeight = 72.5f; // 15 cm from top of head.
	CrouchedEyeHeight = 41.25f;
	USolCharacterMovementComponent* SolMoveComp = Cast<USolCharacterMovementComponent>(GetMovementComponent());
	if (SolMoveComp)
	{
		SolMoveComp->CrouchedHalfHeight = 87.5f; //56.25f;
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
	WeaponAttachPoint = "HandRSocket"; //"GunSocketTest";
	HelmetAttachPoint = "HelmetSocket";
	RightThighAttachPoint = "ThighRSocket";

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

	InventoryComponent = ObjectInitializer.CreateDefaultSubobject<UInventoryComponent>(this, TEXT("InventoryComponent"));
	InventoryComponent->SetMaxInventoryMass(10.f);

	/** There are nine different body sections so far. */
	BodySections.SetNum(9);
	BodySections[EBodySection::Head].Bones.Add("head");
	BodySections[EBodySection::Head].Bones.Add("neck_01");

	BodySections[EBodySection::UpperTorso].Bones.Add("spine_03");
	BodySections[EBodySection::UpperTorso].Bones.Add("spine_02");

	BodySections[EBodySection::LowerTorso].Bones.Add("pelvis");
	BodySections[EBodySection::LowerTorso].Bones.Add("spine_01");

	BodySections[EBodySection::LeftTorso].Bones.Add("clavicle_l");

	BodySections[EBodySection::RightTorso].Bones.Add("clavicle_r");

	BodySections[EBodySection::LeftLeg].Bones.Add("thigh_l");
	BodySections[EBodySection::LeftLeg].Bones.Add("calf_l");
	BodySections[EBodySection::LeftLeg].Bones.Add("foot_l");
	BodySections[EBodySection::LeftLeg].Bones.Add("ball_l");

	BodySections[EBodySection::RightLeg].Bones.Add("thigh_r");
	BodySections[EBodySection::RightLeg].Bones.Add("calf_r");
	BodySections[EBodySection::RightLeg].Bones.Add("foot_r");
	BodySections[EBodySection::RightLeg].Bones.Add("ball_r");

	BodySections[EBodySection::LeftArm].Bones.Add("upperarm_l");
	BodySections[EBodySection::LeftArm].Bones.Add("lowerarm_l");
	BodySections[EBodySection::LeftArm].Bones.Add("hand_l");
	BodySections[EBodySection::LeftArm].Bones.Add("index_01_l");
	BodySections[EBodySection::LeftArm].Bones.Add("index_02_l");
	BodySections[EBodySection::LeftArm].Bones.Add("index_03_l");
	BodySections[EBodySection::LeftArm].Bones.Add("middle_01_l");
	BodySections[EBodySection::LeftArm].Bones.Add("middle_02_l");
	BodySections[EBodySection::LeftArm].Bones.Add("middle_03_l");
	BodySections[EBodySection::LeftArm].Bones.Add("ring_01_l");
	BodySections[EBodySection::LeftArm].Bones.Add("ring_02_l");
	BodySections[EBodySection::LeftArm].Bones.Add("ring_03_l");
	BodySections[EBodySection::LeftArm].Bones.Add("pinky_01_l");
	BodySections[EBodySection::LeftArm].Bones.Add("pinky_02_l");
	BodySections[EBodySection::LeftArm].Bones.Add("pinky_03_l");
	BodySections[EBodySection::LeftArm].Bones.Add("thumb_01_l");
	BodySections[EBodySection::LeftArm].Bones.Add("thumb_02_l");
	BodySections[EBodySection::LeftArm].Bones.Add("thumb_03_l");

	BodySections[EBodySection::RightArm].Bones.Add("upperarm_r");
	BodySections[EBodySection::RightArm].Bones.Add("lowerarm_r");
	BodySections[EBodySection::RightArm].Bones.Add("hand_r");
	BodySections[EBodySection::RightArm].Bones.Add("index_01_r");
	BodySections[EBodySection::RightArm].Bones.Add("index_02_r");
	BodySections[EBodySection::RightArm].Bones.Add("index_03_r");
	BodySections[EBodySection::RightArm].Bones.Add("middle_01_r");
	BodySections[EBodySection::RightArm].Bones.Add("middle_02_r");
	BodySections[EBodySection::RightArm].Bones.Add("middle_03_r");
	BodySections[EBodySection::RightArm].Bones.Add("ring_01_r");
	BodySections[EBodySection::RightArm].Bones.Add("ring_02_r");
	BodySections[EBodySection::RightArm].Bones.Add("ring_03_r");
	BodySections[EBodySection::RightArm].Bones.Add("pinky_01_r");
	BodySections[EBodySection::RightArm].Bones.Add("pinky_02_r");
	BodySections[EBodySection::RightArm].Bones.Add("pinky_03_r");
	BodySections[EBodySection::RightArm].Bones.Add("thumb_01_r");
	BodySections[EBodySection::RightArm].Bones.Add("thumb_02_r");
	BodySections[EBodySection::RightArm].Bones.Add("thumb_03_r");

	Health = 100.f;
	FullHealth = 100.f;
	MaxHealth = 150.f;
	SevereDamage = 0.f;
	SevereDamageScalar = 1.0f; //0.125f;
	HealthRegenRate = 1.5f;
	HealthTicksPerSec = 8.f;
	LegHealth = 50.f;
	FullLegHealth = 50.f;

	Stamina = 10.f;
	MaxStamina = 10.f;

	CurrentAimPct = 0.f;
	CurrentZoomPct = 0.f;
	bIsDying = false;
	bWeaponFiringAllowed = true;
	bIsAiming = false;


	bIsZooming = false;
	ZoomInTime = 0.25f;
	ZoomOutTime = 0.125f;
	ZoomScale = 2.5f;


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
	RecoilCurveTime = 0.0f;
	RecoilTimeScalar = 0.5f;

	HeldWeaponOffset = FRotator::ZeroRotator;
	AimBreathingOffset = FRotator::ZeroRotator;
	LastControlRotation = FRotator::ZeroRotator;
	MaxFreeAimRadius = 7.5f; //10.f;
	MaxRecoilOffsetRadius = 10.f;

	WeaponSwayTime = 0.0f;
	BreathingTime = 0.f;
}

void ASolCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

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
			Stamina = FMath::Max(Stamina - (0.9f * DeltaSeconds), 0.f);
			if (Stamina < 0.f)
			{
				SetSprinting(false);
			}
		}
		/**Regenerate stamina if not sprinting **/
		else if (Stamina < MaxStamina)
		{
			Stamina = FMath::Min(Stamina + (1 * DeltaSeconds), MaxStamina);
		}

		/** Adjust the weapon's aim percentage. **/
		if (EquippedItem) //Since the weapon itself has the aim speed.
		{
			float AimSpeed = 0.5f;
			AFirearm* EquippedFirearm = Cast<AFirearm>(EquippedItem);
			if (EquippedFirearm)
			{
				AimSpeed = EquippedFirearm->GetAimSpeed();
			}
			if (bIsAiming)
			{
				if (CurrentAimPct < 1.f)
				{
					CurrentAimPct = FMath::Min(CurrentAimPct + (DeltaSeconds / AimSpeed), 1.f);
				}
			}
			else if (CurrentAimPct > 0.f)
			{
				CurrentAimPct = FMath::Max(CurrentAimPct - (DeltaSeconds / AimSpeed), 0.f);
			}
		}

		/** Add recoil to character if necessary. */
		if (RecoilCurveTime > 0.0f)
		{
			ProcessRecoil(DeltaSeconds);
		}

		// Zoom in if we want to.
		if (bIsZooming)
		{
			if (CurrentZoomPct < 1.f) 
			{
				CurrentZoomPct = FMath::Min(CurrentZoomPct + (DeltaSeconds / ZoomInTime), 1.f);
			}
		}
		// Otherwise, zoom out.
		else if (CurrentZoomPct > 0.f)
		{
			CurrentZoomPct = FMath::Max(CurrentZoomPct - (DeltaSeconds / ZoomOutTime), 0.f);
		}

		AddWeaponSway(DeltaSeconds);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ASolCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("DropWeapon", IE_Pressed, this, &ASolCharacter::OnDropWeapon);

	// Temporary zoom control.
	InputComponent->BindAction("Zoom", IE_Pressed, this, &ASolCharacter::OnStartZoom);
	InputComponent->BindAction("Zoom", IE_Released, this, &ASolCharacter::OnStopZoom);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analogue joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ASolCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ASolCharacter::LookUpAtRate);
}

/** Below functions currently are the input. Those listed above will eventually be relegated to the player controller.
    This will allow us to do actions while dead ("Press [Fire] To Respawn") or to relegate those
	functions to controlled vehicles. **/

void ASolCharacter::OnStartZoom()
{
	bIsZooming = !bIsZooming;
	//bIsZooming = true;
}

void ASolCharacter::OnStopZoom()
{
	//bIsZooming = false;
}

void ASolCharacter::StartFire()
{
	if (EquippedItem && bWeaponFiringAllowed)
	{
		EquippedItem->StartFire();
	}
}

void ASolCharacter::StopFire()
{
	if (EquippedItem)
	{
		EquippedItem->StopFire();
	}
}

void ASolCharacter::OnReload()
{
	if (EquippedItem)
	{
		EquippedItem->StartReload();
	}
}

void ASolCharacter::OnSwitchFireMode()
{
	if (EquippedItem)
	{
		EquippedItem->StartSwitchFireMode();
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
	if (GetInventoryComponent()->GetInventoryCount() >= 1 && (EquippedItem == NULL || EquippedItem->GetWeaponState() != "Equipping"))
	{
		TArray<AInventoryItem*> Inventory = GetInventoryComponent()->GetInventory();
		const int32 EquippedItemIdx = Inventory.IndexOfByKey(EquippedItem);
		AInventoryItem* NewItem = Inventory[FMath::Abs(EquippedItemIdx + DeltaIndex) % Inventory.Num()];
		if (NewItem && NewItem->CanBeEquipped() && NewItem != EquippedItem)
		{
			SetPendingWeapon(NewItem); //EquipItem(NewWeapon);
		}
	}
}

void ASolCharacter::OnDropWeapon()
{
	if (EquippedItem)
	{
		DropInventory(EquippedItem);
	}
}

void ASolCharacter::OnStartUse()
{
	HandleUse();
}

void ASolCharacter::OnStopUse()
{
	// Nothing yet.
}

/** Below functions deal with the result of the input. **/

void ASolCharacter::EquipItem(AInventoryItem* Item)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetEquippedWeapon(Item);
	}
	else
	{
		ServerEquipWeapon(Item);
	}
}

bool ASolCharacter::ServerEquipWeapon_Validate(AInventoryItem* Weapon)
{
	return true;
}

void ASolCharacter::ServerEquipWeapon_Implementation(AInventoryItem* Weapon)
{
	EquipItem(Weapon);
}

void ASolCharacter::DropInventory(AInventoryItem* Inv)
{
	// Only allow us to drop the item if it is in our inventory.
	if (Inv && Inv->DroppedPickupClass && GetInventoryComponent()->ContainsItem(Inv))
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			FVector SpawnVector = GetWeaponAimLoc(); //+ (GetWeaponAimRot().RotateVector(FVector(50.f, 0.f, 0.f)));
			FTransform SpawnTM(GetWeaponAimRot(), SpawnVector);
			APickup* DroppedPickup = Cast<APickup>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, APickup::StaticClass(), SpawnTM));
			if (DroppedPickup)
			{
				UE_LOG(LogDamage, Warning, TEXT("%s: Dropping item %s."), *GetName(), *Inv->GetName());
				GetInventoryComponent()->RemoveFromInventory(Inv);
				DroppedPickup->SetHeldItem(Inv);
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

UInteractableComponent* ASolCharacter::FindInteractable(TSubclassOf<UInteractionEvent>& Interaction)
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
			if (AActor* Obj = OutHits[i].GetActor())
			{
				// Get the first InteractableComponent for now.
				if (UInteractableComponent* Interactable = Obj->FindComponentByClass<UInteractableComponent>())
				{
					// Get all interactions and return the first one that we can do.
					TArray<TSubclassOf<UInteractionEvent>> Interactions = Interactable->GetInteractions(this);
					for (int j = 0; j < Interactions.Num(); j++)
					{
						UInteractionEvent* TestInteraction = Interactions[j]->GetDefaultObject<UInteractionEvent>();
						if (TestInteraction && TestInteraction->CanInteract(this, Interactable))
						{
							Interaction = Interactions[j];
							return Interactable;
						}
					}
				}
			}
		}
	}
	return nullptr;
}

void ASolCharacter::HandleUse()
{
	// If we're a client, notify the server.
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerHandleUse();
	}

	bool bUseSuccessful = false;
	TSubclassOf<UInteractionEvent> Interaction = nullptr;
	if (UInteractableComponent* Interactable = FindInteractable(Interaction))
	{
		Interactable->OnStartUseBy(this, Interaction);
		/*
		bUseSuccessful = Interactable->OnStartUseBy(this);
		// Only play sound if local player controller owns this pawn.
		if (PickupItemSound && IsFirstPerson())
		{
			UGameplayStatics::PlaySoundAtLocation(this, PickupItemSound, GetActorLocation());
		}
		if (!bUseSuccessful && UseDenialSound && IsFirstPerson())
		{
			UGameplayStatics::PlaySoundAtLocation(this, UseDenialSound, GetActorLocation());
		}
		*/
	}
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

UInventoryComponent* ASolCharacter::GetInventoryComponent() const
{
	return InventoryComponent;
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

float ASolCharacter::GetFullHealth() const
{
	return FullHealth;
}

float ASolCharacter::GetCappedHealth() const
{
	return FullHealth - SevereDamage;
}

float ASolCharacter::GetMaxHealth() const
{
	return MaxHealth;
}

void ASolCharacter::SetHealth(float NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0.f, MaxHealth);
}

void ASolCharacter::SetFullHealth(float NewFullHealth)
{
	FullHealth = FMath::Max(NewFullHealth, 0.f);
	// Ensure max health is never below full health.
	if (FullHealth > MaxHealth)
	{
		SetMaxHealth(FullHealth);
	}
}

void ASolCharacter::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = FMath::Max(NewMaxHealth, 0.f);
	// Ensure full health is never above max health.
	if (MaxHealth < FullHealth)
	{
		SetFullHealth(MaxHealth);
	}
}

float ASolCharacter::GetLegHealth() const
{
	return LegHealth;
}

float ASolCharacter::GetFullLegHealth() const
{
	return FullLegHealth;
}

AInventoryItem* ASolCharacter::GetEquippedItem() const
{
	return EquippedItem;
}

AInventoryItem* ASolCharacter::GetPendingItem()
{
	return PendingItem;
}

FName ASolCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

FName ASolCharacter::DetermineItemAttachPoint(AInventoryItem* Inv)
{
	/**
	if (Inv == SidearmWeapon)
	{
		return RightThighAttachPoint;
	}
	else if ((Inv == PrimaryWeapon && SecondaryWeapon == EquippedItem) || (Inv == SecondaryWeapon && PrimaryWeapon == EquippedItem))
	{
		return BackAttachPoint;
	}
	*/
	return "None";
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

void ASolCharacter::SetEquippedWeapon(AInventoryItem* NewWeapon, AInventoryItem* LastWeapon)
{
	AInventoryItem* LocalLastWeapon = nullptr;

	if (!LastWeapon)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != EquippedItem)
	{
		LocalLastWeapon = EquippedItem;
	}

	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnequip();
	}
	EquippedItem = NewWeapon;
	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);
		NewWeapon->OnEquip();
	}
}

void ASolCharacter::SetPendingWeapon(AInventoryItem* NewWeapon)
{
	PendingItem = NewWeapon;

	if (EquippedItem)
	{
		EquippedItem->OnUnequip();
	}

	if (!EquippedItem)
	{
		EquipItem(PendingItem);
		PendingItem = nullptr;
	}
}

void ASolCharacter::OnWeaponUnequipFinish(AInventoryItem* OldWeapon)
{
	if (EquippedItem == OldWeapon)
	{
		EquippedItem = nullptr;
		IdleAnimSeq = DefaultIdleAnimSeq;
	}
	if (PendingItem)
	{
		EquipItem(PendingItem);
		PendingItem = nullptr;
	}
}

UAnimSequence* ASolCharacter::GetIdleAnimSequence() const
{
	return IdleAnimSeq;
}

void ASolCharacter::SpawnInitialInventory(TArray<TSubclassOf<AInventoryItem>> DefaultInventoryToAdd, bool bUsePawnDefaultInventory)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}
	// Spawn the pawm's default inventory.
	if (bUsePawnDefaultInventory)
	{
		for (int32 i = 0; i < DefaultInventoryClasses.Num(); i++)
		{
			if (DefaultInventoryClasses[i])
			{
				CreateNewInventoryItem(DefaultInventoryClasses[i]);
			}
		}
	}
	// Spawn inventory given to use from the game mode.
	for (int32 i = 0; i < DefaultInventoryToAdd.Num(); i++)
	{
		if (DefaultInventoryToAdd[i])
		{
			CreateNewInventoryItem(DefaultInventoryToAdd[i]);
		}
	}
	// Equip an item we can wield.
	if (!EquippedItem)
	{
		for (auto InvItem : GetInventoryComponent()->GetInventory())
		{
			if (InvItem && InvItem->CanBeEquipped())
			{
				EquipItem(InvItem);
				break;
			}
		}
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
	GetInventoryComponent()->AddToInventory(NewItem);
	return NewItem;
}

bool ASolCharacter::CanPickUpItem(AInventoryItem* Item) const
{
	return InventoryComponent->CanHoldItem(Item) || InventoryComponent->CanSwapForItem(Item);
}

//////////////////////////////////////////////////////////////////////////
// Weapon Location/Rotation

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
	}
	if (DeltaAimLagPitch != 0 || DeltaAimLagYaw != 0)
	{
		const float AimLagScalar = 0.5f;
		//AddWeaponOffset(FRotator(DeltaAimLagPitch * AimLagScalar, DeltaAimLagYaw * AimLagScalar, 0.0f), MaxFreeAimRadius, MaxFreeAimRadius);
	}

	LastControlRotation = NewControlRotation;
	
	Super::FaceRotation(NewControlRotation, DeltaTime);
}

void ASolCharacter::OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation)
{
	// Move to camera class in the future.
	const float DefaultFOV = 90.f;
	const float AimedFOV = 75.f;
	const float ZoomedFOV = DefaultFOV / ZoomScale;

	const float DesiredZoomFOV = FMath::LerpStable(DefaultFOV, ZoomedFOV, CurrentZoomPct);
	const float DesiredAimFOV = FMath::LerpStable(DefaultFOV, AimedFOV, CurrentAimPct);
	// Pick whichever has the largest zoom.
	FirstPersonCameraComponent->FieldOfView = FMath::Min(DesiredZoomFOV, DesiredAimFOV);

	// END move to camera class.

	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("CharacterMesh1P")));

	FVector MeshLoc = DefMesh1P->GetRelativeLocation();
	float EyeHeightTemp = bIsCrouched ? CrouchedEyeHeight : BaseEyeHeight;
	FVector EyeHeightVector = FVector(0.0f, 0.0f, EyeHeightTemp);
	FirstPersonCameraComponent->SetRelativeLocation(EyeHeightVector);
	FVector TempOffset = FVector(-27.5f, 0.5f, -35.f) + EyeHeightVector; //FVector(-25.f, 1.f, -37.5f);  //23.49, 0, 141.24 vs 0, 0, 165
	// Temp to remove arms when nothing is equipped.
	if (!EquippedItem)
	{
		TempOffset -= FVector(0.f, 0.f, -200.f);
	}
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
	const FRotator FinalRot = HeldWeaponOffset + AimBreathingOffset;
	return FinalRot;
}

FVector ASolCharacter::GetWeaponLocationOffset() const
{
	const AInventoryItem* Weapon = GetEquippedItem();
	FVector AimedVector = FVector::ZeroVector;
	FVector UnaimedVector = FVector::ZeroVector;
	if (Weapon)
	{
		UnaimedVector += Weapon->GetMeshOffset();
		AFirearm* Firearm = Cast<AFirearm>(EquippedItem);
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
	const FVector WeaponOffset = GetWeaponLocationOffset(); // +FVector(75.f, 0.f, -7.5f); //Temporary offset for muzzle.
	const FVector FinalLocation = EyeLocation + GetWeaponAimRot().RotateVector(WeaponOffset);
	return FinalLocation;
}

void ASolCharacter::AddRecoil(FVector InRecoil)
{
	if (RecoilCurveTime <= 0.0f)
	{
		CurrentRecoilVelocity = FVector::ZeroVector;
	}
	CurrentRecoilVelocity += InRecoil;
	RecoilCurveTime = 1.0f;
}

void ASolCharacter::ProcessRecoil(float DeltaSeconds)
{
	if (RecoilCurveTime > 0.0f)
	{
		// Make a pseudo-curve (faster to slower).
		const float ScaledTime = DeltaSeconds / RecoilTimeScalar;
		const float RecoilThisTick = FMath::Clamp(ScaledTime, 0.0f, RecoilCurveTime);

		// Calculate recoil to apply this tick.
		const FVector TotalRecoilThisTick = RecoilThisTick * CurrentRecoilVelocity;

		AddWeaponOffset(FRotator(TotalRecoilThisTick.X, TotalRecoilThisTick.Y, 0.0f), MaxRecoilOffsetRadius * 3.f, MaxRecoilOffsetRadius * 3.f);

		// Finally, update the recoil.
		const float NewRecoilTime = FMath::Max(RecoilCurveTime - RecoilThisTick, 0.0f);
		RecoilCurveTime = NewRecoilTime;
		const float RecoilDampening = FMath::Min(CurrentRecoilVelocity.Size() * DeltaSeconds, 1.f);
		CurrentRecoilVelocity -= CurrentRecoilVelocity * RecoilDampening;
	}
}

void ASolCharacter::AddWeaponSway(float DeltaSeconds)
{
	// Handle bob due to breathing. Happens no matter what.
	float BreathingRate = 1.f;
	float LastBreathingTime = BreathingTime;
	BreathingTime += DeltaSeconds * BreathingRate;

	float BreathPitchSwayValue = FMath::Cos(BreathingTime * 1.875f) - FMath::Cos(LastBreathingTime * 1.875f);
	float BreathYawSwayValue = FMath::Sin(BreathingTime) - FMath::Sin(LastBreathingTime);

	FRotator BreathingSwayRange(0.25f, 0.125f, 0.125f);
	FRotator BreathingSwayChange = FRotator(BreathingSwayRange.Pitch * BreathPitchSwayValue, BreathingSwayRange.Yaw * BreathYawSwayValue, 0);
	AimBreathingOffset += BreathingSwayChange;

	// Handle weapon bob due to movement.
	USolCharacterMovementComponent* SolMoveComp = Cast< USolCharacterMovementComponent>(GetCharacterMovement());
	if (SolMoveComp)
	{
		// If moving, keep swaying left to right.
		if (!SolMoveComp->Velocity.IsZero() && SolMoveComp->IsWalking())
		{
			// Values to probably define elsewhere so they're modifiable.
			FRotator WalkingSwayRange(1.25f, 1.5f, 0.25f);
			FRotator SprintingSwayRange(2.25f, 2.75f, 0.5f);

			// Increase the sway rate if moving on the ground.
			float SpeedRatio = SolMoveComp->IsWalking() ? FMath::Clamp(SolMoveComp->Velocity.Size() / SolMoveComp->MaxWalkSpeed, 0.f, 2.f) : 0.f;
			float LastWeaponSwayTime = WeaponSwayTime;
			WeaponSwayTime += DeltaSeconds * FMath::Max(SpeedRatio * 6.f, 1.f);

			float PitchSwayValue = FMath::Cos(WeaponSwayTime * 2) - FMath::Cos(LastWeaponSwayTime * 2);
			float YawSwayValue = FMath::Sin(WeaponSwayTime) - FMath::Sin(LastWeaponSwayTime);
			FRotator RotToAdd = FRotator::ZeroRotator;
			// Running
			if (SpeedRatio > 1.f) {
				//RotToAdd = FRotator(4.f * PitchSwayValue, 7.5f * YawSwayValue, 0);
				RotToAdd = FRotator(FMath::Lerp(WalkingSwayRange.Pitch, SprintingSwayRange.Pitch, SpeedRatio - 1.f) * PitchSwayValue,
					                FMath::Lerp(WalkingSwayRange.Yaw, SprintingSwayRange.Yaw, SpeedRatio - 1.f) * YawSwayValue,
					                0);
			}
			// Walking
			else
			{
				RotToAdd = FRotator(FMath::Lerp(0.f, WalkingSwayRange.Pitch, SpeedRatio) * PitchSwayValue,
					                FMath::Lerp(0.f, WalkingSwayRange.Yaw, SpeedRatio) * YawSwayValue,
										0);
			}
			AddWeaponOffset(RotToAdd, MaxFreeAimRadius, MaxFreeAimRadius);
		}
		// Otherwise, return to the centre of the screen.
		else
		{
			WeaponSwayTime = 0.f;
			HeldWeaponOffset *= FMath::Max(0.0f, 1.0f - DeltaSeconds);
		}
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

void ASolCharacter::MoveUp(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorUpVector(), Value);
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

	// Let inventory (e.g. armour) modify incoming damage.
	for (auto InvItem : GetInventoryComponent()->GetInventory())
	{
		InvItem->ModifyDamageTaken(Damage, DamageEvent.DamageTypeClass);
	}

	bool TEST_bHitInLegs = false;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID) || DamageEvent.IsOfType(FSolPointDamageEvent::ClassID))
	{
		//(const FPointDamageEvent&)DamageEvent
		FHitResult HitInfo;
		FVector Dir;
		// The API reference for FDamageEvent says this will "ideally" go away after time.
		// That said, this is so far the only way I can figure out how to retrieve the HitInfo, since "casting" doesn't work.
		DamageEvent.GetBestHitInfo(this, DamageCauser, HitInfo, Dir); //(const FPointDamageEvent&)DamageEvent.GetBestHitInfo.BoneName;
		FName HitBone = HitInfo.BoneName;

		// This could be done more modularly, but this works for now.
		float DamagePctAtZero = 1.f;
		float DamagePctAtEnd = 1.f;
		float DamageCurveEnd = GetFullHealth();
		// Head and neck
		if (BodySections[EBodySection::Head].Bones.Contains(HitBone))
		{
			DamagePctAtEnd = 4.f;
			DamageCurveEnd = 25.f;
		}
		// Upper body
		else if (BodySections[EBodySection::UpperTorso].Bones.Contains(HitBone))
		{
			// No damage scaling
		}
		// Pelvis && stomach
		else if (BodySections[EBodySection::LowerTorso].Bones.Contains(HitBone))
		{
			DamagePctAtZero = 1.f;
			DamagePctAtEnd = 0.875f;
		}
		// Legs
		else if (BodySections[EBodySection::LeftLeg].Bones.Contains(HitBone) || BodySections[EBodySection::RightLeg].Bones.Contains(HitBone))
		{
			DamagePctAtZero = 0.875f;
			DamagePctAtEnd = 0.625f;
			TEST_bHitInLegs = true;
		}
		// Arms (currently identical to being hit in the legs)
		else if (BodySections[EBodySection::LeftArm].Bones.Contains(HitBone) || BodySections[EBodySection::RightArm].Bones.Contains(HitBone))
		{
			DamagePctAtZero = 0.875f;
			DamagePctAtEnd = 0.625f;
		}
		// Shoulders? (clavicle)
		else if (BodySections[EBodySection::LeftTorso].Bones.Contains(HitBone) || BodySections[EBodySection::RightTorso].Bones.Contains(HitBone))
		{
			DamagePctAtZero = 0.875f;
			DamagePctAtEnd = 0.75f;
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("TakeDamage(): Where was %s hit?!"), *this->GetName()));
		}
		const float DamageScalar = FMath::Lerp(DamagePctAtZero, DamagePctAtEnd, FMath::Clamp(Damage / DamageCurveEnd, 0.f, 1.f));
		Damage *= DamageScalar;
	}

	const float DamageToHealth = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (DamageToHealth > 0.f)
	{
		Health -= DamageToHealth;
		SevereDamage += DamageToHealth * SevereDamageScalar;

		if (TEST_bHitInLegs)
		{
			LegHealth -= FMath::Min(LegHealth, DamageToHealth);
		}

		if (Health <= 0)
		{
			
			Die(DamageToHealth, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(DamageToHealth, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
			// Start health regen if we are not doing so already.
			if (Health < FullHealth && !GetWorldTimerManager().IsTimerActive(TimerHandle_RegenHealth))
			{
				GetWorldTimerManager().SetTimer(TimerHandle_RegenHealth, this, &ASolCharacter::RegenHealth, 1 / HealthTicksPerSec, true);
			}
		}
		//MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}
	return Damage;
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

	// Drop any inventory we can and destroy the rest.
	if (GetLocalRole() == ROLE_Authority)
	{
		if (EquippedItem)
		{
			DropInventory(EquippedItem);
		}
		while (GetInventoryComponent()->GetInventoryCount() > 0)
		{
			// Roundabout way of dropping the first item in the inventory.
			DropInventory(GetInventoryComponent()->GetInventory()[0]);
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

		GetInventoryComponent()->DestroyInventory();
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
		LegHealth = FMath::Min(FullLegHealth, LegHealth + HealthToRegen);
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
		// TODO: Add landing view bob.
		//HeldWeaponOffset.Pitch += FMath::Max(0.f, FMath::Abs(GetCharacterMovement()->Velocity.Z) * 0.125f);
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
	
	if (GetPlayerState<ASolPlayerState>())
	{
		PrimaryColor = GetPlayerState<ASolPlayerState>()->GetPrimaryColor();
		ATeamState* MyTeam = GetPlayerState<ASolPlayerState>()->GetTeam();
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

	// everyone except local owner: flag change is locally instigated
	DOREPLIFETIME_CONDITION(ASolCharacter, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASolCharacter, bIsSprinting, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ASolCharacter, LastTakeHitInfo, COND_Custom);

	// everyone
	DOREPLIFETIME(ASolCharacter, EquippedItem);
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

void ASolCharacter::OnRep_EquippedItem(class AInventoryItem* LastWeapon)
{
	SetEquippedWeapon(EquippedItem, LastWeapon);
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
