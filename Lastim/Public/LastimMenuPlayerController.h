// Copyright Kyle Taylor Lange

#pragma once
#include "LastimPlayerController.h"
#include "LastimMenuPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ALastimMenuPlayerController : public ALastimPlayerController
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