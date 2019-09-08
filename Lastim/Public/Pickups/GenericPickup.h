// Copyright Kyle Taylor Lange

#pragma once

#include "Pickup.h"
#include "GenericPickup.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AGenericPickup : public APickup
{
	GENERATED_BODY()

public:

	AGenericPickup(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	// Returns a default copy of the InventoryClass we hold.
	class ALastimInventory* GetHeldItem() const override;

	// Runs CreatePickupMesh on client.
	UFUNCTION()
	virtual void OnRep_InventoryClass();

	// Override to create an instance of InventoryClass for the pawn.
	virtual void GivePickupTo(class ALastimCharacter* Pawn) override;

	UFUNCTION(BlueprintCallable, Category = Pickup)
	virtual void SetInventoryClass(TSubclassOf<class ALastimInventory> InClass);

protected:

	// Class of pickup to spawn. Set using SetInventoryClass.
	UPROPERTY(EditDefaultsOnly, Replicated, ReplicatedUsing = OnRep_InventoryClass)
	TSubclassOf<class ALastimInventory> InventoryClass;
};
