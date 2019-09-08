// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimCharacter.h"
#include "LastimInventory.h"

//////////////////////////////////////////////////////////////////////////
// ALastimInventory

ALastimInventory::ALastimInventory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("LastimInventory", "ItemName", "Unknown Item");
	DroppedPickupClass = ADroppedPickup::StaticClass();
	bReplicates = true;
}

void ALastimInventory::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

FString ALastimInventory::GetDisplayName() const
{
	return DisplayName.ToString();
}

void ALastimInventory::OnEnterInventory(ALastimCharacter* NewOwner)
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(NewOwner);
	}
}

void ALastimInventory::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}
}

void ALastimInventory::SetOwningPawn(ALastimCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

ALastimCharacter* ALastimInventory::GetOwningPawn()
{
	return MyPawn;
}

// We'll later make this actually return something for LastimInventory, but for now we just care about making it work for LastimWeapon.
UMeshComponent* ALastimInventory::GetPickupMesh()
{
	return nullptr;
}

void ALastimInventory::Destroyed()
{
	if (MyPawn)
	{
		MyPawn->RemoveFromInventory(this);
		GEngine->AddOnScreenDebugMessage(-1, 600.f, FColor::Red, FString::Printf(TEXT("~~~~%s: Item in inventory before destruction!~~~~"), *this->GetName()));
		UE_LOG(LogDamage, Warning, TEXT("DESTROYED(): %s still has owner!"), *this->GetName());

		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("%s: Item in %s inventory before destruction!"), *GetName(), *MyPawn->GetName()));
	}
	Super::Destroyed();
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ALastimInventory::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALastimInventory, MyPawn);
}

void ALastimInventory::OnRep_MyPawn()
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
