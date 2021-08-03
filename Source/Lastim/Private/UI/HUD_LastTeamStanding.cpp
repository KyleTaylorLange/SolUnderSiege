// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "TeamState.h"
#include "SolPlayerState.h"
#include "SolGameState.h"
#include "HUD_LastTeamStanding.h"

AHUD_LastTeamStanding::AHUD_LastTeamStanding(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//bDrawLivesInScoreboard = true;
}

void AHUD_LastTeamStanding::DrawGameData(FVector2D &DrawPosition, ASolGameState* InGameState)
{
	Super::DrawGameData(DrawPosition, InGameState);
}
