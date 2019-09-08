// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "LastimSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ALastimSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
protected:

	/** Update controls to use the same input bindings as defined in the project. */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	
};
