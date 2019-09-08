// Copyright Kyle Taylor Lange

#pragma once

#include "LastimInventory.h"
#include "InventoryBag.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AInventoryBag : public ALastimInventory
{
	GENERATED_UCLASS_BODY()

public:
	// List of held items.
	TArray<ALastimInventory*> ContainedItems;

	virtual UMeshComponent* GetPickupMesh() override;

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh3P;
};
