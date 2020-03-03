// Fill out your copyright notice in the Description page of Project Settings.

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "Weapon.h"

//////////////////////////////////////////////////////////////////////////
// AWeapon

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weapon", "ItemName", "Unknown Weapon");
	
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	//Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	RootComponent = Mesh1P;

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	//Mesh3P->bChartDistanceFactor = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetupAttachment(Mesh1P);

	bIsEquipped = false;
	bPendingEquip = false;
	bPendingUnequip = false;
	CurrentState = "Idle";
	WeaponSlotType = WeaponSlotType::Equipment;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	DetachMeshFromPawn();

	/** Create material instances so we can manipulate them later (for ammo indicators and such). **/
	for (int32 iMat = 0; iMat < Mesh1P->GetNumMaterials(); iMat++) //(int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(Mesh1P->CreateAndSetMaterialInstanceDynamic(iMat));
	}
}

void AWeapon::StartFire()
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}
	**/
}

void AWeapon::StopFire()
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopFire();
	}
	**/
}

void AWeapon::StartReload(bool bFromReplication)
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartReload();
	}
	**/
}

void AWeapon::StopReload()
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (GetLocalRole() < ROLE_Authority)
	{
	ServerStopReload();
	}
	**/
}

void AWeapon::StartSwitchFireMode(bool bFromReplication)
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (GetLocalRole() < ROLE_Authority)
	{
	ServerStartReload();
	}
	**/
}

void AWeapon::OnEquip()
{
	AttachMeshToPawn();

	bPendingEquip = true;
	DetermineWeaponState();

	EquipDuration = PlayWeaponAnimation(EquipAnim, EquipAnim);
	if (EquipDuration <= 0.0f)
	{
		// failsafe
		EquipDuration = 0.5f;
	}
	EquipStartedTime = GetWorld()->GetTimeSeconds();

	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AWeapon::OnEquipFinished, EquipDuration, false);

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		//PlayWeaponSound(EquipSound);
	}
}

void AWeapon::OnEquipFinished()
{
	AttachMeshToPawn();
	bIsEquipped = true;
	bPendingEquip = false;
	DetermineWeaponState();

	//PlayWeaponAnimation(IdleAnim1P, IdleAnim3P);
}

void AWeapon::OnUnequip()
{
	StopFire();

	if (CurrentState == "Idle")
	{
		AttachMeshToPawn();

		bPendingUnequip = true;
		DetermineWeaponState();

		UnequipDuration = PlayWeaponAnimation(UnequipAnim, UnequipAnim);
		if (UnequipDuration <= 0.0f)
		{
			// failsafe
			UnequipDuration = 0.5f;
		}
		UnequipStartedTime = GetWorld()->GetTimeSeconds();

		GetWorldTimerManager().SetTimer(TimerHandle_OnUnequipFinished, this, &AWeapon::OnUnequipFinished, UnequipDuration, false);

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			//PlayWeaponSound(UnequipSound);
		}
	}
}

void AWeapon::OnUnequipFinished()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	bPendingUnequip = false;
	DetermineWeaponState();

	if (MyPawn)
	{
		MyPawn->OnWeaponUnequipFinish(this);
	}
}

/** Mesh Functions. **/

void AWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s->IsLocallyControlled(): %s"), *MyPawn->GetName(), MyPawn->IsLocallyControlled() ? TEXT("true") : TEXT("false") ));
		////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s->IsFirstPerson(): %s"), *MyPawn->GetName(), MyPawn->IsFirstPerson() ? TEXT("true") : TEXT("false")));
		if (MyPawn->IsFirstPerson() == true) //(MyPawn->IsLocallyControlled() == true) 
		{
			USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecificPawnMesh(true);
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetSpecificPawnMesh(false);
			Mesh1P->SetHiddenInGame(false);
			Mesh3P->SetHiddenInGame(true);
			Mesh1P->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			Mesh3P->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
	    else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
			UseWeaponMesh->SetHiddenInGame(false);
		}
	}
}

void AWeapon::DetachMeshFromPawn()
{
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->SetHiddenInGame(true);
}

void AWeapon::OnEnterInventory(ASolCharacter* NewOwner)
{
	Super::OnEnterInventory(NewOwner);
}

void AWeapon::OnLeaveInventory()
{
	Super::OnLeaveInventory();
	// Also ensure weapon is unequipped.
	OnUnequipFinished();
}

FName AWeapon::GetWeaponState()
{
	return CurrentState;
}

void AWeapon::SetWeaponState(FName NewState)
{
	//const FName PrevState = CurrentState;
	CurrentState = NewState;
}

void AWeapon::DetermineWeaponState()
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

FRotator AWeapon::GetAdjustedAimRot() const
{
	FRotator FinalRot;
	if (MyPawn)
	{
		FinalRot= MyPawn->GetWeaponAimRot();
	}
	return FinalRot;
}

FVector AWeapon::GetAdjustedAimLoc() const
{
	FVector FinalLoc;
	if (MyPawn)
	{
		FinalLoc = MyPawn->GetWeaponAimLoc() + (MyPawn->GetWeaponAimRot().Vector() * 20.f);
	}
	return FinalLoc;
}

float AWeapon::GetAIRating()
{
	return BaseAIRating;
}

UMeshComponent* AWeapon::GetPickupMesh()
{
	if (Mesh3P)
	{
		// We'd prefer to get this weapon's actual mesh, but for now we'll get the default one.
		USkeletalMeshComponent* OutMesh = GetClass()->GetDefaultObject<AWeapon>()->Mesh3P;
		return OutMesh;
	}
	else
	{
		return Super::GetPickupMesh();
	}
}

USkeletalMeshComponent* AWeapon::GetMesh1P() const 
{
	return Mesh1P;
}

USkeletalMeshComponent* AWeapon::GetMesh3P() const
{
	return Mesh3P;
}

USkeletalMeshComponent* AWeapon::GetWeaponMesh() const
{
	return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

FVector AWeapon::GetWeaponOffset() const
{
	return WeaponOffset;
}

//////////////////////////////////////////////////////////////////////////
// Animation

float AWeapon::PlayWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P) //(const FWeaponAnim& Animation)
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

void AWeapon::StopWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P) //(const FWeaponAnim& Animation)
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

//////////////////////////////////////////////////////////////////////////
// Replication

/* We'll probably need this later, but for now it doesn't compile.
void AWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
*/

bool AWeapon::ServerStartFire_Validate()
{
	return true;
}

void AWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AWeapon::ServerStopFire_Validate()
{
	return true;
}

void AWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AWeapon::ServerStartReload_Validate()
{
	return true;
}

void AWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool AWeapon::ServerStopReload_Validate()
{
	return true;
}

void AWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

bool AWeapon::ServerStartSwitchFireMode_Validate()
{
	return true;
}

void AWeapon::ServerStartSwitchFireMode_Implementation()
{
	StartSwitchFireMode();
}