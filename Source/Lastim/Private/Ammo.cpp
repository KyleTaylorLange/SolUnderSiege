// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "Ammo.h"

AAmmo::AAmmo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo", "AmmoName", "Unknown Ammo Type");
	
	AmmoCount = 25;
	MaxAmmo = 25;
	AmmoLostOnUnload = 1;
	RechargeAmount = 0;

	// Temp mesh just so it is visible.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelMesh(TEXT("/Game/Character/UE4_Mannequin/Mesh/SK_Mannequin_1PArms.SK_Mannequin_1PArms"));

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("AmmoMesh"));
	Mesh3P->SetSkeletalMesh(SkelMesh.Object);
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->SetVisibility(false); // TEMP. TODO: Properly handle mesh visibility (when to hide and show).

	bReplicates = true;
}

/* Might use later to recharge ammo.
void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

FString AAmmo::GetDisplayName() const
{
	// Used to do stuff here.
	return Super::GetDisplayName();
}

int32 AAmmo::GetAmmoCount() const
{
	return AmmoCount;
}

int32 AAmmo::GetMaxAmmo() const
{
	return MaxAmmo;
}

void AAmmo::SetMaxAmmo(int32 NewMaxAmmo)
{
	MaxAmmo = NewMaxAmmo;
}

int32 AAmmo::UseAmmo(int32 InAmmo)
{
	if (AmmoCount > 0)
	{
		const int32 UsedAmmo = FMath::Min(InAmmo, AmmoCount);
		AmmoCount -= UsedAmmo;
		// Begin recharging ammo.
		if (RechargesAmmo() && AmmoCount < MaxAmmo)
		{
			if (GetWorldTimerManager().IsTimerActive(TimerHandle_AmmoRecharge) == false || 
				GetWorldTimerManager().GetTimerRemaining(TimerHandle_AmmoRecharge) > CalculateNextRechargeTime())
			{
				GetWorldTimerManager().SetTimer(TimerHandle_AmmoRecharge, this, &AAmmo::OnAmmoRecharge, CalculateNextRechargeTime());
			}
		}
		return UsedAmmo;
	}
	return 0;
}

int32 AAmmo::AddAmmo(int32 InAmmo)
{
	if (InAmmo > 0)
	{
		const int32 AddedAmmo = FMath::Min(InAmmo, MaxAmmo - AmmoCount);
		AmmoCount += AddedAmmo;
		return AddedAmmo;
	}
	return 0;
}

bool AAmmo::RechargesAmmo() const
{
	return RechargeRateBracket.Num() > 0 && RechargeAmount > 0;
}

void AAmmo::OnAmmoRecharge()
{
	if (RechargesAmmo() && AmmoCount < MaxAmmo)
	{
		AmmoCount += FMath::Min(RechargeAmount, MaxAmmo - AmmoCount);
		// Keep recharging if we're not full, otherwise end the timer.
		if (AmmoCount < MaxAmmo)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_AmmoRecharge, this, &AAmmo::OnAmmoRecharge, CalculateNextRechargeTime());
		}
		else
		{
			GetWorldTimerManager().ClearTimer(TimerHandle_AmmoRecharge);
		}
	}
}

float AAmmo::CalculateNextRechargeTime() const
{
	if (RechargeAmount > 0)
	{
		float RechargeRate = 0;
		for (int32 i = 0; i < RechargeRateBracket.Num(); i++)
		{
			if (GetAmmoCount() < RechargeRateBracket[i].Y || RechargeRateBracket[i].Y < 0)
			{
				if (RechargeRate < RechargeRateBracket[i].X)
				{
					RechargeRate = RechargeRateBracket[i].X;
				}
			}
		}
		return RechargeAmount / RechargeRate;
	}
	return 0.f;
}


void AAmmo::OnLoadIntoWeapon()
{

}

void AAmmo::OnUnloadFromWeapon()
{
	// Lose some ammo on reload.
	UseAmmo(AmmoLostOnUnload);
	// Destroy if empty.
	if (AmmoCount <= 0 && !RechargesAmmo())
	{
		ASolCharacter* LOwner = GetOwningPawn();
		if (LOwner)
		{
			LOwner->RemoveFromInventory(this);
		}
		Destroy();
	}
}

void AAmmo::OnEnterInventory(class ASolCharacter* NewOwner)
{
	// We used to do stuff here.
	Super::OnEnterInventory(NewOwner);
}

UMeshComponent* AAmmo::GetPickupMesh()
{
	if (Mesh3P)
	{
		USkeletalMeshComponent* OutMesh = GetClass()->GetDefaultObject<AAmmo>()->Mesh3P;
		OutMesh->SetVisibility(true); //TEMP until we properly handle mesh visibility.
		return OutMesh;
	}
	else
	{
		return Super::GetPickupMesh();
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication

void AAmmo::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner
	DOREPLIFETIME_CONDITION(AAmmo, AmmoCount, COND_OwnerOnly);
}