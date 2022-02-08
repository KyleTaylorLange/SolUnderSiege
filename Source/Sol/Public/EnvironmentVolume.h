// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PhysicsVolume.h"
#include "EnvironmentVolume.generated.h"

/**
 * Represents the variables of a new environment different from the rest of the level, including: gravity, temperature, and pressure
 */
UCLASS()
class SOL_API AEnvironmentVolume : public APhysicsVolume
{
	GENERATED_BODY()

public:

	AEnvironmentVolume(const FObjectInitializer& ObjectInitializer);

	virtual float GetGravityZ() const override;

	virtual float GetTemperature() const;

	virtual float GetPressure() const;

protected:

	// Overrides gravity if not zero. 1000 = Earth's gravity.
	UPROPERTY(EditAnywhere, Category = Environment)
	float GravityZ;

	// Overrides temperature (in Kelvin) if not zero.
	UPROPERTY(EditAnywhere, Category = Environment)
	float Temperature;

	// Overrides pressure in atmospheres if above zero.
	UPROPERTY(EditAnywhere, Category = Environment)
	float Pressure;
	
};
