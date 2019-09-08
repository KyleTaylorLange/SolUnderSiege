// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "Player/LastimAIController.h"
#include "AI_Invasion.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AAI_Invasion : public ALastimAIController
{
	GENERATED_BODY()
	
	virtual bool IsEnemy(APawn* InPawn) override;
	
};
