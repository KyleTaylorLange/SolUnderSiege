// Copyright Kyle Taylor Lange

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "SolPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API ASolPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()

public:

	/** Player's default FOV. */
	float NormalFOV;

	/** Player's FOV when aiming. */
	float AimingFOV;

	/** Player's FOV when sprinting. */
	float SprintingFOV;

	/** The minimum of the player's FOV when zoomed in. */
	float MinZoomFOV;

	bool bLimitViewRotation;

	/** After updating camera, inform pawn to update 1p mesh to match camera's location&rotation */
	virtual void UpdateCamera(float DeltaTime) override;

	virtual void ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot) override;
};