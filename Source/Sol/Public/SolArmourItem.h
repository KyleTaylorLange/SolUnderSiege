// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "InventoryItem.h"
#include "SolArmourItem.generated.h"

/**
 * Parent class of any item that absorbs damage.
 */
UCLASS()
class SOL_API ASolArmourItem : public AInventoryItem
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable)
	virtual void ModifyDamageTaken(float& Damage, TSubclassOf<UDamageType> DamageType) override;

	UFUNCTION(BlueprintCallable)
	float GetArmour();

	UFUNCTION(BlueprintCallable)
	void SetArmour(float Armour);

protected:

	/** Current amount of armour left. */
	UPROPERTY(EditAnywhere)
	float Armour;

	/** The total armour this device can hold. */
	UPROPERTY(EditAnywhere)
	float MaxArmour;

	/** The percentage of damage absorbed (after the initial damage absorbed). */
	UPROPERTY(EditAnywhere)
	float DamageAbsorptionPct;
};
