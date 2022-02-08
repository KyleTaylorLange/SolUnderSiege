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
	GENERATED_BODY()
	
public:

	AGameMode_Anarchy(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void DetermineMatchWinner() override;

	virtual void CheckForMatchWinner() override;
};
