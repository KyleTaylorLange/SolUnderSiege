// Copyright Kyle Taylor Lange

#pragma once

#include "SolBot.h"
#include "Bot_Domination.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API ABot_Domination : public ASolBot
{
	GENERATED_BODY()

	virtual bool CheckObjective() override;
	
};
