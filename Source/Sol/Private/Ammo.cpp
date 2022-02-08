// Copyright Kyle Taylor Lange

#include "Sol.h"
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
	bLoadedInWeapon = false;

	// Temp mesh just so it is visible.
	//static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelMesh(TEXT("/Game/Character/UE4_Mannequin/Mesh/SK_Mannequin_1PArms.SK_Mannequin_1PArms"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TempStaticMesh(TEXT("/Game/ShooterGameEffects/Meshes/Gameplay/Pickups/Ammo.Ammo"));

	UStaticMeshComponent* StaticPickupMesh = Cast<UStaticMeshComponent>(PickupMesh);
	//UStaticMeshComponent* Mesh3PStatic = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("AmmoMesh"));
	StaticPickupMesh->SetStaticMesh(TempStaticMesh.Object);
	//PickupMesh = Mesh3PStatic;
	//Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	//PickupMesh->SetVisibility(false); // TEMP. TODO: Properly handle mesh visibility (when to hide and show).

	bReplicates = true;
}

/* Might use later to recharge ammo.
void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

FString AAmmo::GetDisplayName() const
{
	if (GetMaxAmmo() > 0)
	{
		return Super::GetDisplayName() + FString(" (") + FString::FromInt(100 * GetAmmoCount() / GetMaxAmmo()) + FString("%)");
	}
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

float AAmmo::UseAmmo(float InAmmo)
{
	if (AmmoCount > 0)
	{
		const float UsedAmmo = FMath::Min(InAmmo, (float)AmmoCount);
		AmmoCount -= UsedAmmo;
		// Begin recharging ammo.
		if (!bLoadedInWeapon && RechargesAmmo() && AmmoCount < MaxAmmo)
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

float AAmmo::AddAmmo(float InAmmo)
{
	if (InAmmo > 0)
	{
		const float AddedAmmo = FMath::Min(InAmmo, (float)(MaxAmmo - AmmoCount));
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
	bLoadedInWeapon = true;
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_AmmoRecharge))
	{
		GetWorldTimerManager().PauseTimer(TimerHandle_AmmoRecharge);
	}
}

void AAmmo::OnUnloadFromWeapon()
{
	bLoadedInWeapon = false;
	// Destroy if empty.
	if (RechargesAmmo() && GetAmmoCount() < GetMaxAmmo())
	{
		if (GetWorldTimerManager().IsTimerActive(TimerHandle_AmmoRecharge) == false ||
			GetWorldTimerManager().GetTimerRemaining(TimerHandle_AmmoRecharge) > CalculateNextRechargeTime())
		{
			GetWorldTimerManager().SetTimer(TimerHandle_AmmoRecharge, this, &AAmmo::OnAmmoRecharge, CalculateNextRechargeTime());
		}
	}
	else if (AmmoCount <= 0)
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
	// Temp test to consolidate ammo.
	if (NewOwner && !RechargesAmmo())
	{
		for (int32 i = 0; i < NewOwner->ItemInventory.Num(); i++)
		{
			AAmmo* OtherAmmo = Cast<AAmmo>(NewOwner->ItemInventory[i]);
			if (OtherAmmo && OtherAmmo != this && OtherAmmo->GetClass() == this->GetClass())
			{
				AAmmo* Transferer;
				AAmmo* Transferee;
				if (OtherAmmo->GetAmmoCount() > this->GetAmmoCount())
				{
					Transferer = this;
					Transferee = OtherAmmo;
				}
				else
				{
					Transferer = OtherAmmo;
					Transferee = this;
				}
				int32 AmmoToTransfer = FMath::Min(Transferer->GetAmmoCount(), Transferee->GetMaxAmmo() - Transferee->GetAmmoCount());
				Transferer->UseAmmo(AmmoToTransfer);
				Transferee->AddAmmo(AmmoToTransfer);
				if (Transferer->GetAmmoCount() <= 0)
				{
					NewOwner->RemoveFromInventory(Transferer);
					Transferer->Destroy();
				}
			}
		}
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