// Copyright Kyle Taylor Lange

#pragma once

#include "TeamGameMode.h"
#include "GameMode_TeamAnarchy.generated.h"

/**
 * TEAM ANARCHY
 *  Team deathmatch: you are on a team, and you gain points for killing anyone who isn't on your team.
 *  There can be multiple teams, each a different colour (later we can add "factions" instead of coloured teams as an option).
 */
UCLASS()
class SOL_API AGameMode_TeamAnarchy : public ATeamGameMode
{
	GENERATED_BODY()
	
public:

	AGameMode_TeamAnarchy(const FObjectInitializer& ObjectInitializer);
};
