// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "SolWorldSettings.generated.h"

/**
 * Includes global world settings unique to Sol.
 */
UCLASS()
class LASTIM_API ASolWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

	ASolWorldSettings(const FObjectInitializer& ObjectInitializer);

public:

	float GetTemperature() const;

	float GetPressure() const;

protected:

	// Temperature for the whole level in Kelvin.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Physics)
	float GlobalTemperature;

	// Atmospheric pressure for the whole level in (Earth) atmospheres.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Physics)
	float GlobalPressure;
	
};
