// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "LastimDestructible.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ALastimDestructible : public AActor
{
	GENERATED_BODY()
	
public:

	ALastimDestructible(const FObjectInitializer& ObjectInitializer);

	/** Item's health. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Health;

	/** Item's armour. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Armour;

	/** Take damage and handle destruction. */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
	
};
