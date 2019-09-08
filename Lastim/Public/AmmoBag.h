// Copyright Kyle Taylor Lange

#pragma once

#include "InventoryBag.h"
#include "AmmoBag.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AAmmoBag : public AInventoryBag
{
	GENERATED_BODY()
	
	virtual FString GetDisplayName() const override;
	
	virtual void OnEnterInventory(ALastimCharacter* NewOwner) override;
};
