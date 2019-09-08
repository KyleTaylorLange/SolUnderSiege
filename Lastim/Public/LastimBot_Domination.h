// Copyright Kyle Taylor Lange

#pragma once

#include "LastimBot.h"
#include "LastimBot_Domination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ALastimBot_Domination : public ALastimBot
{
	GENERATED_BODY()

	virtual bool CheckObjective() override;
	
};
