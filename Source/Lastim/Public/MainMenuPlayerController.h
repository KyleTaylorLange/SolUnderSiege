// Copyright Kyle Taylor Lange

#pragma once
#include "SolPlayerController.h"
#include "MainMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AMainMenuPlayerController : public ASolPlayerController
{
	GENERATED_UCLASS_BODY()
	
public:

	// Begin Controller interface
	/** We know we won't have a pawn, so we don't care */
	virtual void FailedToSpawnPawn() override {}
	// End Controller interface

protected:

	//virtual void SetupInputComponent() override;
	
};