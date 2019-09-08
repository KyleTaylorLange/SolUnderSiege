// Fill out your copyright notice in the Description page of Project Settings.

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimCharacter.h"
#include "LastimWeapon.h"

//////////////////////////////////////////////////////////////////////////
// ALastimWeapon

ALastimWeapon::ALastimWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("LastimWeapon", "ItemName", "Unknown Weapon");
	
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

void ALastimWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	DetachMeshFromPawn();

	/** Create material instances so we can manipulate them later (for ammo indicators and such). **/
	for (int32 iMat = 0; iMat < Mesh1P->GetNumMaterials(); iMat++) //(int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(Mesh1P->CreateAndSetMaterialInstanceDynamic(iMat));
	}
}

void ALastimWeapon::StartFire()
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}
	**/
}

void ALastimWeapon::StopFire()
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (Role < ROLE_Authority)
	{
		ServerStopFire();
	}
	**/
}

void ALastimWeapon::StartReload(bool bFromReplication)
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (Role < ROLE_Authority)
	{
		ServerStartReload();
	}
	**/
}

void ALastimWeapon::StopReload()
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (Role < ROLE_Authority)
	{
	ServerStopReload();
	}
	**/
}

void ALastimWeapon::StartSwitchFireMode(bool bFromReplication)
{
	/** Just a friendly reminder to include this in subclasses that need it.
	if (Role < ROLE_Authority)
	{
	ServerStartReload();
	}
	**/
}

void ALastimWeapon::OnEquip()
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

	GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &ALastimWeapon::OnEquipFinished, EquipDuration, false);

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		//PlayWeaponSound(EquipSound);
	}
}

void ALastimWeapon::OnEquipFinished()
{
	AttachMeshToPawn();
	bIsEquipped = true;
	bPendingEquip = false;
	DetermineWeaponState();

	//PlayWeaponAnimation(IdleAnim1P, IdleAnim3P);
}

void ALastimWeapon::OnUnequip()
{
	// For now, still do instant unequipping.
	StopFire();
	//OnUnequipFinished();
	//return; 

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

		GetWorldTimerManager().SetTimer(TimerHandle_OnUnequipFinished, this, &ALastimWeapon::OnUnequipFinished, UnequipDuration, false);

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			//PlayWeaponSound(UnequipSound);
		}
	}
}

void ALastimWeapon::OnUnequipFinished()
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

void ALastimWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint();
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s->IsLocallyControlled(): %s"), *MyPawn->GetName(), MyPawn->IsLocallyControlled() ? TEXT("true") : TEXT("false") ));
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s->IsFirstPerson(): %s"), *MyPawn->GetName(), MyPawn->IsFirstPerson() ? TEXT("true") : TEXT("false")));
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

void ALastimWeapon::DetachMeshFromPawn()
{
	Mesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh1P->SetHiddenInGame(true);

	Mesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	Mesh3P->SetHiddenInGame(true);
}

void ALastimWeapon::OnEnterInventory(ALastimCharacter* NewOwner)
{
	Super::OnEnterInventory(NewOwner);
}

void ALastimWeapon::OnLeaveInventory()
{
	Super::OnLeaveInventory();
	// Also ensure weapon is unequipped.
	OnUnequipFinished();
}

FName ALastimWeapon::GetWeaponState()
{
	return CurrentState;
}

void ALastimWeapon::SetWeaponState(FName NewState)
{
	//const FName PrevState = CurrentState;
	CurrentState = NewState;
}

void ALastimWeapon::DetermineWeaponState()
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

FRotator ALastimWeapon::GetAdjustedAimRot() const
{
	FRotator FinalRot;
	if (MyPawn)
	{
		FinalRot= MyPawn->GetWeaponAimRot();
	}
	return FinalRot;
}

FVector ALastimWeapon::GetAdjustedAimLoc() const
{
	FVector FinalLoc;
	if (MyPawn)
	{
		FinalLoc = MyPawn->GetWeaponAimLoc() + (MyPawn->GetWeaponAimRot().Vector() * 20.f);
	}
	return FinalLoc;
}

float ALastimWeapon::GetAIRating()
{
	return BaseAIRating;
}

UMeshComponent* ALastimWeapon::GetPickupMesh()
{
	if (Mesh3P)
	{
		// We'd prefer to get this weapon's actual mesh, but for now we'll get the default one.
		USkeletalMeshComponent* OutMesh = GetClass()->GetDefaultObject<ALastimWeapon>()->Mesh3P;
		return OutMesh;
	}
	else
	{
		return Super::GetPickupMesh();
	}
}

USkeletalMeshComponent* ALastimWeapon::GetMesh1P() const 
{
	return Mesh1P;
}

USkeletalMeshComponent* ALastimWeapon::GetMesh3P() const
{
	return Mesh3P;
}

USkeletalMeshComponent* ALastimWeapon::GetWeaponMesh() const
{
	return (MyPawn != NULL && MyPawn->IsFirstPerson()) ? Mesh1P : Mesh3P;
}

FVector ALastimWeapon::GetWeaponOffset() const
{
	return WeaponOffset;
}

//////////////////////////////////////////////////////////////////////////
// Animation

float ALastimWeapon::PlayWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P) //(const FWeaponAnim& Animation)
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

void ALastimWeapon::StopWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P) //(const FWeaponAnim& Animation)
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
void ALastimWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
*/

bool ALastimWeapon::ServerStartFire_Validate()
{
	return true;
}

void ALastimWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool ALastimWeapon::ServerStopFire_Validate()
{
	return true;
}

void ALastimWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool ALastimWeapon::ServerStartReload_Validate()
{
	return true;
}

void ALastimWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool ALastimWeapon::ServerStopReload_Validate()
{
	return true;
}

void ALastimWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

bool ALastimWeapon::ServerStartSwitchFireMode_Validate()
{
	return true;
}

void ALastimWeapon::ServerStartSwitchFireMode_Implementation()
{
	StartSwitchFireMode();
}