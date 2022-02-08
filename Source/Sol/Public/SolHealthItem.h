// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "SolHealthItem.generated.h"

/**
 * Parent class of any item that heals health upon use.
 */
UCLASS()
class SOL_API ASolHealthItem : public AInventoryItem
{
	GENERATED_UCLASS_BODY()

	virtual void StartFire() override;

	virtual void StopFire() override;
	
protected:

	/** Raw health regained when used. */
	UPROPERTY(EditDefaultsOnly)
	float HealthNumRegained;

	/** Percentage of missing health regained in addition to raw health regained.  */
	UPROPERTY(EditDefaultsOnly)
	float HealthPctRegained;
};
