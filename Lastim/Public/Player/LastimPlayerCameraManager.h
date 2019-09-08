// Copyright Kyle Taylor Lange

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "LastimPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ALastimPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()

public:

	/** normal FOV */
	float NormalFOV;

	/** targeting FOV */
	float AimingFOV;

	/** After updating camera, inform pawn to update 1p mesh to match camera's location&rotation */
	virtual void UpdateCamera(float DeltaTime) override;
};