// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "SolArmourItem.h"
#include "BattleRoyaleShield.generated.h"

/**
 * Item that will slowly damage the player after a certain time unless recharged.
 */
UCLASS()
class SOL_API ABattleRoyaleShield : public ASolArmourItem
{
	GENERATED_UCLASS_BODY()
	
	virtual void ModifyDamageTaken(float& Damage, TSubclassOf<UDamageType> DamageType) override;

	virtual void OnEnterInventory(ASolCharacter* NewOwner) override;

	virtual void OnLeaveInventory() override;

	virtual void OnDrain();

	virtual float CalculateNextDrainTime();

	FString GetDisplayName() const override;

protected:
	UPROPERTY()
	bool bModifiesDamage;

	// How much damage to do per drain tick.
	float DrainPerTick;

	// The time between each drain tick.
	float TimeBetweenDrainTicks;

	// Timer handle for the drain function.
	FTimerHandle TimerHandle_Drain;
};
