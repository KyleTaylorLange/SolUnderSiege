// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "GameMode_CaptureTheFlag.h"

AGameMode_CaptureTheFlag::AGameMode_CaptureTheFlag(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_CaptureTheFlag", "CaptureTheFlag", "Capture The Flag");

	/* Personal scores. */
	KillScore = 1;
	TeamkillScore = -1;
	DeathScore = 0;
	SuicideScore = -1;
	/* Only score from flag captures. */
	KillScoreForTeam = 0;
	TeamkillScoreForTeam = 0;
	DeathScoreForTeam = 0;
	SuicideScoreForTeam = 0;
}


