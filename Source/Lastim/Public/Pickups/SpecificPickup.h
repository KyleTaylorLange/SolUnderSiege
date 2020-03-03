// Copyright Kyle Taylor Lange

#pragma once

#include "Pickup.h"
#include "SpecificPickup.generated.h"

/**
 * TODO: Merge me into my parent class.
 *       The dichotomy between "GenericPickup" (where it just has a class of item to spawn) and 
 *       "SpecificPickup" has mostly been abandoned in favour of spawning an item for every pickup, so
 *       there's no point in keeping it separate from its parent.
 */
UCLASS()
class LASTIM_API ASpecificPickup : public APickup
{
	GENERATED_BODY()
	
public:

	// Assigns an inventory item to this pickup.
	//UFUNCTION(BlueprintImplementableEvent)
	virtual void AssignItemToPickup(class AInventoryItem* InItem);

	// Override to transfer the HeldItem to the pawn.
	virtual void GivePickupTo(class ASolCharacter* Pawn) override;

	// For SpecificWeapon, actually gets HeldItem.
	class AInventoryItem* GetHeldItem() const override;

	// Item this pickup holds.
	UPROPERTY(BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_HeldItem)
	class AInventoryItem* HeldItem;

	// Runs CreatePickupMesh on client.
	UFUNCTION()
	virtual void OnRep_HeldItem();

	// Override to destroy any held items.
	virtual void Destroyed() override;
};
