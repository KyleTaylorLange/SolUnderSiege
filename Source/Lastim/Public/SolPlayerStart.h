// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/PlayerStart.h"
#include "SolPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ASolPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:

	/** Team number for this player spawn. Zero is no team. */
	UPROPERTY(EditInstanceOnly, Category = PlayerStart)
	int8 SpawnTeam;
};
