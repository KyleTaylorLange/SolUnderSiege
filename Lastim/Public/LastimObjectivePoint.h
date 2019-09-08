// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Info.h" //#include "GameFramework/Actor.h"
#include "LastimObjectivePoint.generated.h"

UCLASS()
class LASTIM_API ALastimObjectivePoint : public AInfo //AActor
{
	GENERATED_BODY()
	
public:	

	ALastimObjectivePoint(const FObjectInitializer& ObjectInitializer);

	/* Team number for this objective. Zero for no team affiliation. */
	UPROPERTY(EditInstanceOnly, Category = Objective)
	int8 TeamIndex;

	/* Is this a primary team objective, such as a flag? */
	UPROPERTY(EditInstanceOnly, Category = Objective)
	bool bPrimaryObjective;
};
