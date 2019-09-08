// Copyright Kyle Taylor Lange

#pragma once

#include "SpecificPickup.h"
#include "DroppedPickup.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ADroppedPickup : public ASpecificPickup
{
	GENERATED_BODY()
	
public:

	ADroppedPickup(const FObjectInitializer& ObjectInitializer);

	// Currently overridden to set a 60 second lifetime.
	//Todo?: maybe only set a life span if there are too many pickups in the level?
	//Also todo: check the game mode for a proper life span; we probably don't want to delete pickups on singleplayer/cooperative/etc gametypes.
	virtual void BeginPlay() override;
	
};
