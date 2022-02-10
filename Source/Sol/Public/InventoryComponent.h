// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

/**
 * The InventoryComponent handles a collection of inventory items held by
 * an actor in the game world. This can be a player pawn or a static object
 * like a supply crate or store.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOL_API UInventoryComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Adds item to the inventory.
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual void AddToInventory(class AInventoryItem* NewItem, class AInventoryItem* OldItem = nullptr);

	/** Removes item from inventory. */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	virtual bool RemoveFromInventory(class AInventoryItem* ItemToRemove);

	/** 
	 * Checks if the player can hold an inventory item (not including swapping for one).
	 * @param Item The item to be added to the inventory.
	 * @return True if the inventory can hold the item.
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool CanHoldItem(class AInventoryItem* Item) const;

	/**
	 * Does this inventory component contain this item?
	 * @param Item The Item to search for.
	 * @return True if present in this inventory.
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool ContainsItem(class AInventoryItem* Item) const;

	/**
	 * @return All inventory items.
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	TArray<class AInventoryItem*> GetInventory() const;

	// Checks if the player can pick up this item by swapping for it.
	// Returns a pointer to the item they would swap for.
	UFUNCTION(BlueprintCallable, Category = Inventory)
	class AInventoryItem* CanSwapForItem(class AInventoryItem* Item) const;

	/** Returns length of the player's inventory. **/
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	int32 GetInventoryCount() const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	float GetCurrentInventoryMass() const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	float GetMaxInventoryMass() const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void SetMaxInventoryMass(float MaxMass);

	// Destroys all inventory items.
	UFUNCTION(BlueprintCallable)
	virtual void DestroyInventory();

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	/** List of inventory items carried by this character. */
	UPROPERTY(Transient, Replicated)
	TArray<class AInventoryItem*> ItemInventory;

	/** The current amount of mass in this inventory. */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	float CurrentInventoryMass;

	/** The maximum mass the inventory can hold. */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	float DefaultInventoryMassCapacity;
};
