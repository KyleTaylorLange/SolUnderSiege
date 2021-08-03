// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "InventoryItem.h"

//////////////////////////////////////////////////////////////////////////
// AInventoryItem

AInventoryItem::AInventoryItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("InventoryItem", "ItemName", "Unknown Item");
	DroppedPickupClass = ADroppedPickup::StaticClass();
	MassInKG = 0.5f;
	InventoryMassScalar = 1.f;
	bCanBeEquipped = true;
	bReplicates = true;

	bIsEquipped = false;
	bPendingEquip = false;
	bPendingUnequip = false;
	CurrentState = "Idle";

	UStaticMeshComponent* StaticMeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AmmoMesh"));
	PickupMesh = StaticMeshComp;
	PickupMesh->SetVisibility(false); // TEMP. TODO: Properly handle mesh visibility (when to hide and show).
}

void AInventoryItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	DetachMeshFromPawn();
}

FString AInventoryItem::GetDisplayName() const
{
	return DisplayName.ToString();
}

float AInventoryItem::GetMassInKG() const
{
	return MassInKG;
}

float AInventoryItem::GetMassInInventory() const
{
	return MassInKG * InventoryMassScalar;
}

bool AInventoryItem::CanBeEquipped() const
{
	return bCanBeEquipped;
}

void AInventoryItem::OnEnterInventory(ASolCharacter* NewOwner)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetOwningPawn(NewOwner);
	}
}

void AInventoryItem::OnLeaveInventory()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetOwningPawn(NULL);
		// Also ensure weapon is unequipped.
		OnUnequipFinished();
	}
}

void AInventoryItem::OnEquip()
{
	if (MyPawn)
	{
		AttachMeshToPawn(MyPawn->GetWeaponAttachPoint());
	}

	bPendingEquip = true;
	DetermineWeaponState();

	EquipDuration = PlayWeaponAnimation(EquipAnim, EquipAnim);
	if (EquipDuration <= 0.0f)
	{
		// failsafe
		EquipDuration = 0.5f;
	}
	EquipStartedTime = GetWorld()->GetTimeSeconds();

	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AInventoryItem::OnEquipFinished, EquipDuration, false);

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		//PlayWeaponSound(EquipSound);
	}
}

void AInventoryItem::OnEquipFinished()
{
	if (MyPawn)
	{
		AttachMeshToPawn(MyPawn->GetWeaponAttachPoint());
		// TEST: Change idle anim pose.
		//MyPawn->IdleAnimSeq = IdleAnimSeq;
	}
	bIsEquipped = true;
	bPendingEquip = false;
	DetermineWeaponState();

	//PlayWeaponAnimation(IdleAnim1P, IdleAnim3P);
}

void AInventoryItem::OnUnequip()
{
	StopFire();

	if (CurrentState == "Idle")
	{
		if (MyPawn)
		{
			AttachMeshToPawn(MyPawn->GetWeaponAttachPoint());
		}

		bPendingUnequip = true;
		DetermineWeaponState();

		UnequipDuration = PlayWeaponAnimation(UnequipAnim, UnequipAnim);
		if (UnequipDuration <= 0.0f)
		{
			// failsafe
			UnequipDuration = 0.5f;
		}
		UnequipStartedTime = GetWorld()->GetTimeSeconds();

		GetWorldTimerManager().SetTimer(TimerHandle_OnUnequipFinished, this, &AInventoryItem::OnUnequipFinished, UnequipDuration, false);

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			//PlayWeaponSound(UnequipSound);
		}
	}
}

void AInventoryItem::OnUnequipFinished()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	bPendingUnequip = false;
	DetermineWeaponState();

	if (MyPawn)
	{
		MyPawn->OnWeaponUnequipFinish(this);
		FName AttachPoint = MyPawn->DetermineItemAttachPoint(this);
		if (AttachPoint != "")
		{
			AttachMeshToPawn(AttachPoint);
		}
	}
}

FName AInventoryItem::GetWeaponState() const
{
	return CurrentState;
}

void AInventoryItem::SetWeaponState(FName NewState)
{
	//const FName PrevState = CurrentState;
	CurrentState = NewState;
}

void AInventoryItem::DetermineWeaponState()
{
	FName NewState = "Idle";
	if (bIsEquipped)
	{
		NewState = "Idle";
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

void AInventoryItem::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}
}

void AInventoryItem::StopFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopFire();
	}
}

void AInventoryItem::StartReload(bool bFromReplication)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartReload();
	}
}

void AInventoryItem::StopReload()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopReload();
	}
}

void AInventoryItem::StartSwitchFireMode(bool bFromReplication)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartSwitchFireMode();
	}
}

void AInventoryItem::StopSwitchFireMode()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopSwitchFireMode();
	}
}

void AInventoryItem::SetOwningPawn(ASolCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

ASolCharacter* AInventoryItem::GetOwningPawn()
{
	return MyPawn;
}


USkeletalMeshComponent* AInventoryItem::GetMesh1P() const
{
	return Mesh1P;
}

USkeletalMeshComponent* AInventoryItem::GetMesh3P() const
{
	return Mesh3P;
}

UMeshComponent* AInventoryItem::GetPickupMesh()
{
	if (Mesh3P)
	{
		// We'd prefer to get this weapon's actual mesh, but for now we'll get the default one.
		USkeletalMeshComponent* OutMesh = GetClass()->GetDefaultObject<AInventoryItem>()->Mesh3P;
		return OutMesh;
	}
	else if (PickupMesh)
	{
		UMeshComponent* OutMesh = GetClass()->GetDefaultObject<AInventoryItem>()->PickupMesh;
		OutMesh->SetVisibility(true); //TEMP until we properly handle mesh visibility.
		return OutMesh;
	}
	return nullptr;
}

USkeletalMeshComponent* AInventoryItem::GetMesh() const
{
	return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

void AInventoryItem::AttachMeshToPawn(FName AttachPoint)
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		if (MyPawn->IsFirstPerson())
		{
			if (Mesh1P)
			{
				USkeletalMeshComponent* PawnMesh1P = MyPawn->GetSpecificPawnMesh(true);
				Mesh1P->SetHiddenInGame(false);
				Mesh1P->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			}
			if (Mesh3P)
			{
				USkeletalMeshComponent* PawnMesh1P = MyPawn->GetSpecificPawnMesh(true);
				Mesh1P->SetHiddenInGame(false);
				Mesh1P->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			}
		}
		else
		{
			USkeletalMeshComponent* UseItemMesh = GetMesh();
			if (UseItemMesh)
			{
				USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
				UseItemMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
				UseItemMesh->SetHiddenInGame(false);
			}
		}
	}
}

void AInventoryItem::DetachMeshFromPawn()
{
	if (Mesh1P)
	{
		Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Mesh1P->SetHiddenInGame(true);
	}
	if (Mesh3P)
	{
		Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		Mesh3P->SetHiddenInGame(true);
	}
}

float AInventoryItem::PlayWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P) //(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Anim1P : Anim3P; // Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void AInventoryItem::StopWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P) //(const FWeaponAnim& Animation)
{
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Anim1P : Anim3P; // Animation.Pawn1P : Animation.Pawn3P;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

void AInventoryItem::ModifyDamageTaken(float& Damage, TSubclassOf<UDamageType> DamageType)
{
	// Do nothing in the default implementation.
}

FVector AInventoryItem::GetMeshOffset() const
{
	return MeshOffset;
}

void AInventoryItem::Destroyed()
{
	if (MyPawn)
	{
		MyPawn->RemoveFromInventory(this);
		//GEngine->AddOnScreenDebugMessage(-1, 600.f, FColor::Red, FString::Printf(TEXT("~~~~%s: Item in inventory before destruction!~~~~"), *this->GetName()));
		UE_LOG(LogDamage, Warning, TEXT("DESTROYED(): %s still has owner!"), *this->GetName());

		////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("%s: Item in %s inventory before destruction!"), *GetName(), *MyPawn->GetName()));
	}
	Super::Destroyed();
}

//////////////////////////////////////////////////////////////////////////
// Replication

void AInventoryItem::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInventoryItem, MyPawn);
}

void AInventoryItem::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}

bool AInventoryItem::ServerStartFire_Validate()
{
	return true;
}

void AInventoryItem::ServerStartFire_Implementation()
{
	StartFire();
}

bool AInventoryItem::ServerStopFire_Validate()
{
	return true;
}

void AInventoryItem::ServerStopFire_Implementation()
{
	StopFire();
}

bool AInventoryItem::ServerStartReload_Validate()
{
	return true;
}

void AInventoryItem::ServerStartReload_Implementation()
{
	StartReload();
}

bool AInventoryItem::ServerStopReload_Validate()
{
	return true;
}

void AInventoryItem::ServerStopReload_Implementation()
{
	StopReload();
}

bool AInventoryItem::ServerStartSwitchFireMode_Validate()
{
	return true;
}

void AInventoryItem::ServerStartSwitchFireMode_Implementation()
{
	StartSwitchFireMode();
}

bool AInventoryItem::ServerStopSwitchFireMode_Validate()
{
	return true;
}

void AInventoryItem::ServerStopSwitchFireMode_Implementation()
{
	StopSwitchFireMode();
}
