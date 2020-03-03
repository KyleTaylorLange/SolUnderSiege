// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "DroppedPickup.h"
#include "InventoryItem.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class LASTIM_API AInventoryItem : public AActor
{
	GENERATED_BODY()

public:

	AInventoryItem(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual FString GetDisplayName() const;
	
	// Action called when item enters the player's inventory.
	virtual void OnEnterInventory(class ASolCharacter* NewOwner);
	// Action called when item leaves the player's inventory.
	virtual void OnLeaveInventory();

	// Sets the inventory item's owner.
	void SetOwningPawn(class ASolCharacter* NewOwner);
	// Retrieves the inventory item's owner.
	ASolCharacter* GetOwningPawn();

	// Gets the mesh a pickup of this inventory item should use.
	virtual UMeshComponent* GetPickupMesh();

	// Class this inventory item should use for dropped pickups.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ADroppedPickup> DroppedPickupClass;

	// When item is destroyed. Overridden to inform the Pickup Spawner.
	virtual void Destroyed() override;

protected:

	// Human readable name for this item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	FText DisplayName;

	// The inventory item's owner.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	class ASolCharacter* MyPawn;

	/** ================ **/
	/** SERVER FUNCTIONS **/
	/** ================ **/
	UFUNCTION()
	void OnRep_MyPawn();
};
