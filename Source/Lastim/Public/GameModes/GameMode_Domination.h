// Copyright Kyle Taylor Lange

#pragma once

#include "TeamGameMode.h"
#include "GameMode_Domination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AGameMode_Domination : public ATeamGameMode
{
	GENERATED_UCLASS_BODY()
	
protected:

	// Called when a control point "scores".
	UFUNCTION()
	virtual void OnScoreCP(class ADominationControlPoint* InCP);

	/* Overridden to spawn control points. */
	virtual void ProcessObjectivePoint(class AObjectivePoint* InOP) override;

};
