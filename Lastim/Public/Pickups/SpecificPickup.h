// Copyright Kyle Taylor Lange

#pragma once

#include "Pickup.h"
#include "SpecificPickup.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ASpecificPickup : public APickup
{
	GENERATED_BODY()
	
public:

	// Assigns an inventory item to this pickup.
	//UFUNCTION(BlueprintImplementableEvent)
	virtual void AssignItemToPickup(class ALastimInventory* InItem);

	// Override to transfer the HeldItem to the pawn.
	virtual void GivePickupTo(class ALastimCharacter* Pawn) override;

	// For SpecificWeapon, actually gets HeldItem.
	class ALastimInventory* GetHeldItem() const override;

	// Item this pickup holds.
	UPROPERTY(BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_HeldItem)
	class ALastimInventory* HeldItem;

	// Runs CreatePickupMesh on client.
	UFUNCTION()
	virtual void OnRep_HeldItem();

	// Override to destroy any held items.
	virtual void Destroyed() override;
};
