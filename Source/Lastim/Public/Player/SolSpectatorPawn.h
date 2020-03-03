// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "SolSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ASolSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
protected:

	/** Update controls to use the same input bindings as defined in the project. */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	
};
