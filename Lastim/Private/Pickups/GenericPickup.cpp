// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimInventory.h"
#include "GenericPickup.h"

//////////////////////////////////////////////////////////////////////////
// AGenericPickup
//   Pickup class that does not create a a specific inventory item attached.
//   Rather, it spawns the item upon being picked up.
//////////////////////////////////////////////////////////////////////////

AGenericPickup::AGenericPickup(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AGenericPickup::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Role == ROLE_Authority && InventoryClass)
	{
		CreatePickupMesh(InventoryClass->GetDefaultObject<ALastimInventory>());
	}
}

ALastimInventory* AGenericPickup::GetHeldItem() const
{
	// Returns a default object.
	return InventoryClass->GetDefaultObject<ALastimInventory>();
}

void AGenericPickup::GivePickupTo(class ALastimCharacter* Pawn)
{
	Pawn->CreateNewInventoryItem(InventoryClass);
}

void AGenericPickup::SetInventoryClass(TSubclassOf<class ALastimInventory> InClass)
{
	InventoryClass = InClass;
	if (Role == ROLE_Authority && InventoryClass)
	{
		CreatePickupMesh(InventoryClass->GetDefaultObject<ALastimInventory>());
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication

void AGenericPickup::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGenericPickup, InventoryClass);
}

void AGenericPickup::OnRep_InventoryClass()
{
	CreatePickupMesh(InventoryClass->GetDefaultObject<ALastimInventory>());
}
