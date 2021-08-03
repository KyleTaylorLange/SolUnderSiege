// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Info.h"
#include "ObjectivePoint.generated.h"

// Class for noting important points on the map for game modes.
// These can be items like flag bases for CTF, control points for domination, etc.
UCLASS()
class LASTIM_API AObjectivePoint : public AInfo
{
	GENERATED_BODY()
	
public:	

	AObjectivePoint(const FObjectInitializer& ObjectInitializer);

	/* Team number for this objective. Zero for no team affiliation. */
	UPROPERTY(EditInstanceOnly, Category = Objective)
	int8 TeamIndex;

	/* Is this a primary team objective, such as a flag? */
	UPROPERTY(EditInstanceOnly, Category = Objective)
	bool bPrimaryObjective;
};
