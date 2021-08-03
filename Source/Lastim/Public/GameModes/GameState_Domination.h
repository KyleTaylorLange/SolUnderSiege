// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "SolGameState.h"
#include "GameState_Domination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AGameState_Domination : public ASolGameState
{
	GENERATED_BODY()

public:
	/* List of control points. */
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	TArray<class ADominationControlPoint*> ControlPoints;
};
