// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "SolSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API ASolSpectatorPawn : public ASpectatorPawn
{
	GENERATED_UCLASS_BODY()
	
protected:

	/** Update controls to use the same input bindings as defined in the project. */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	
public:

	/** Moves the pawn up relative to its current location/rotation. */
	virtual void MoveUp(float Value);
};
