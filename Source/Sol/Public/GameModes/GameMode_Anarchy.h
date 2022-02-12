// Copyright Kyle Taylor Lange

#pragma once

#include "SolGameMode.h"
#include "GameMode_Anarchy.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AGameMode_Anarchy : public ASolGameMode
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void DetermineMatchWinner_Implementation() override;

	virtual void CheckForMatchWinner_Implementation() override;
};
