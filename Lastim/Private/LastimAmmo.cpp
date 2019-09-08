// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimCharacter.h"
#include "LastimAmmo.h"

ALastimAmmo::ALastimAmmo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("LastimAmmo", "AmmoName", "Unknown Ammo Type");
	
	AmmoCount = 25;
	MaxAmmo = 25;
	AmmoLostOnUnload = 1;

	bIsLooseAmmo = false;
	RechargeAmount = 0;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelMesh(TEXT("/Game/Character/UE4_Mannequin/Mesh/SK_Mannequin_1PArms.SK_Mannequin_1PArms"));

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("AmmoMesh"));
	Mesh3P->SetSkeletalMesh(SkelMesh.Object);
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	bReplicates = true;
}

/* Might use later to recharge ammo.
void ALastimAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

FString ALastimAmmo::GetDisplayName() const
{
	if (bIsLooseAmmo)
	{
		return FString::Printf(TEXT("%s x%d"), *this->GetDisplayName(), this->GetAmmoCount());

	}
	return Super::GetDisplayName();
}

int32 ALastimAmmo::GetAmmoCount() const
{
	return AmmoCount;
}

int32 ALastimAmmo::GetMaxAmmo() const
{
	return MaxAmmo;
}

void ALastimAmmo::SetMaxAmmo(int32 NewMaxAmmo)
{
	MaxAmmo = NewMaxAmmo;
}

int32 ALastimAmmo::UseAmmo(int32 InAmmo)
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
				GetWorldTimerManager().SetTimer(TimerHandle_AmmoRecharge, this, &ALastimAmmo::OnAmmoRecharge, CalculateNextRechargeTime());
			}
		}
		return UsedAmmo;
	}
	return 0;
}

int32 ALastimAmmo::AddAmmo(int32 InAmmo)
{
	if (InAmmo > 0)
	{
		const int32 AddedAmmo = FMath::Min(InAmmo, MaxAmmo - AmmoCount);
		AmmoCount += AddedAmmo;
		return AddedAmmo;
	}
	return 0;
}

bool ALastimAmmo::RechargesAmmo() const
{
	return RechargeRateBracket.Num() > 0 && RechargeAmount > 0;
}

void ALastimAmmo::OnAmmoRecharge()
{
	if (RechargesAmmo() && AmmoCount < MaxAmmo)
	{
		AmmoCount += FMath::Min(RechargeAmount, MaxAmmo - AmmoCount);
		// Keep recharging if we're not full, otherwise end the timer.
		if (AmmoCount < MaxAmmo)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_AmmoRecharge, this, &ALastimAmmo::OnAmmoRecharge, CalculateNextRechargeTime());
		}
		else
		{
			GetWorldTimerManager().ClearTimer(TimerHandle_AmmoRecharge);
		}
	}
}

float ALastimAmmo::CalculateNextRechargeTime() const
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


void ALastimAmmo::OnLoadIntoWeapon()
{

}

void ALastimAmmo::OnUnloadFromWeapon()
{
	// Lose some ammo on reload.
	UseAmmo(AmmoLostOnUnload);
	// Destroy if empty.
	if (AmmoCount <= 0 && !RechargesAmmo())
	{
		ALastimCharacter* LOwner = GetOwningPawn();
		if (LOwner)
		{
			LOwner->RemoveFromInventory(this);
		}
		Destroy();
	}
}

void ALastimAmmo::OnEnterInventory(class ALastimCharacter* NewOwner)
{
	Super::OnEnterInventory(NewOwner);
	// Check to see if this is extra ammo, then destroy it.
	/*
	if (NewOwner && bIsLooseAmmo)
	{
		for (int32 i = 0; i < NewOwner->ItemInventory.Num(); i++)
		{
			ALastimAmmo* AmmoItem = Cast<ALastimAmmo>(NewOwner->ItemInventory[i]);
			if (AmmoItem && AmmoItem != this && AmmoItem->Tags.Num() > 0 && this->Tags.Num() > 0 && AmmoItem->Tags[0] == this->Tags[0])
			{
				int32 TransferredAmmo = AmmoItem->AddAmmo(AmmoCount);
				AmmoCount -= TransferredAmmo;
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("AmmoItem added %d ammo to item %s, with %d left over!"), TransferredAmmo, *AmmoItem->GetName(), AmmoCount));
				if (AmmoCount <= 0)
				{
					//NewOwner->RemoveFromInventory(this);
					//SetLifeSpan(2.5f);
					//break;
				}
			}
		}
	}
	*/
}

UMeshComponent* ALastimAmmo::GetPickupMesh()
{
	if (Mesh3P)
	{
		USkeletalMeshComponent* OutMesh = GetClass()->GetDefaultObject<ALastimAmmo>()->Mesh3P;
		return OutMesh;
	}
	else
	{
		return Super::GetPickupMesh();
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ALastimAmmo::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner
	DOREPLIFETIME_CONDITION(ALastimAmmo, AmmoCount, COND_OwnerOnly);
}