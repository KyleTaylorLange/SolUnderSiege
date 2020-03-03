// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolGameState.h"
#include "TeamState.h"
#include "GameMode_TeamAnarchy.h"

AGameMode_TeamAnarchy::AGameMode_TeamAnarchy(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_TeamAnarchy", "TeamAnarchy", "Team Anarchy");

	KillScore = 1;
	KillScoreForTeam = 1;
	DeathScore = 0;
	DeathScoreForTeam = 0;
	SuicideScore = -1;
	SuicideScoreForTeam = -1;
	TeamkillScore = -1;
	TeamkillScoreForTeam = -1;

	bDelayedStart = true;
}