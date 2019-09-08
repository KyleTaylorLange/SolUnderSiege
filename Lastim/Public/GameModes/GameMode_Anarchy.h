// Copyright Kyle Taylor Lange

#pragma once

#include "LastimGameMode.h"
#include "GameMode_Anarchy.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AGameMode_Anarchy : public ALastimGameMode
{
	GENERATED_BODY()
	
public:

	AGameMode_Anarchy(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void DetermineMatchWinner() override;

	virtual void CheckForMatchWinner() override;
};
