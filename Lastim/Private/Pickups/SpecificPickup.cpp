// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimInventory.h"
#include "LastimCharacter.h"
#include "UnrealNetwork.h"
#include "SpecificPickup.h"

//////////////////////////////////////////////////////////////////////////
// ASpecificPickup
//   Pickup class that has an already existing LastimInventory object connected to it.
//   This pickup does not create any new items.
//////////////////////////////////////////////////////////////////////////

ALastimInventory* ASpecificPickup::GetHeldItem() const
{
	return HeldItem;
}

void ASpecificPickup::AssignItemToPickup(class ALastimInventory* InItem)
{
	HeldItem = InItem;
	if (HeldItem)
	{
		CreatePickupMesh(HeldItem);
	}
}

void ASpecificPickup::GivePickupTo(class ALastimCharacter* Pawn)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("Pickup: Part IIIa")));
	if (HeldItem)
	{
		Pawn->AddToInventory(HeldItem);
		// Set item to null so it doesn't get destroyed when pickup is destroyed!
		HeldItem = nullptr;
	}
}

void ASpecificPickup::Destroyed()
{
	// Destroy held item before destroying the pickup.
	if (HeldItem)
	{
		HeldItem->Destroy();
	}
	Super::Destroyed();
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ASpecificPickup::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpecificPickup, HeldItem);
}

void ASpecificPickup::OnRep_HeldItem()
{
	CreatePickupMesh(HeldItem);
}