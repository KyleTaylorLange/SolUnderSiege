// Copyright Kyle Taylor Lange

#pragma once

#include "InventoryItem.h"
#include "InventoryBag.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AInventoryBag : public AInventoryItem
{
	GENERATED_UCLASS_BODY()

public:
	// List of held items.
	TArray<AInventoryItem*> ContainedItems;
};
