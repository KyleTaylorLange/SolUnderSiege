// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimGameState.h"
#include "LastimPlayerState.h"
#include "LastimPlayerController.h"
#include "TeamState.h"
#include "GameMode_LastTeamStanding.h"

AGameMode_LastTeamStanding::AGameMode_LastTeamStanding(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_LastTeamStanding", "LastTeamStanding", "Last Team Standing");

	KillScore = 1;
	KillScoreForTeam = 0;
	TeamkillScore = -1;
	TeamkillScoreForTeam = 0;
	DeathScore = -1;
	DeathScoreForTeam = -1;
	SuicideScore = -1;
	SuicideScoreForTeam = -1;

	bDelayedStart = true;
}

void AGameMode_LastTeamStanding::HandleMatchHasStarted()
{
	// Always force respawn, despite server option.
	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->bForceRespawn = true;
	}
	Super::HandleMatchHasStarted();
}

void AGameMode_LastTeamStanding::InitTeamState(ATeamState* InTeam, FTeamProfile InProfile)
{
	Super::InitTeamState(InTeam, InProfile);
	if (InTeam)
	{
		InTeam->AddScore(ScoreLimit);
	}
}

void AGameMode_LastTeamStanding::RestartPlayer(AController* NewPlayer)
{
	// Return if we are out of lives, and go into spectator mode. Maybe not the best implementation?
	ALastimPlayerState* PS = Cast<ALastimPlayerState>(NewPlayer->PlayerState);
	if (PS)
	{
		/* Change this later to GetTeam() */
		int32 MyTeamNum = PS->GetTeamNum();
		ALastimGameState* MyGameState = Cast<ALastimGameState>(GameState);
		if (MyGameState->TeamArray.Num() > MyTeamNum &&
			AliveTeamMemberCount.Num() > MyTeamNum &&
			MyGameState->TeamArray[MyTeamNum]->GetScore() <= AliveTeamMemberCount[MyTeamNum])
		{
			ALastimPlayerController* LPC = Cast<ALastimPlayerController>(NewPlayer);
			if (LPC)
			{
				LPC->BeginWaiting();
			}
			return;
		}
	}
	Super::RestartPlayer(NewPlayer);
	// If we got this far, add us to our team's live player count.
	if (PS)
	{
		if (AliveTeamMemberCount.Num() <= PS->GetTeamNum())
		{
			AliveTeamMemberCount.AddZeroed(PS->GetTeamNum() + 1 - AliveTeamMemberCount.Num());
		}
		AliveTeamMemberCount[PS->GetTeamNum()]++;
	}
}

void AGameMode_LastTeamStanding::OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::OnPlayerDeath(Killer, KilledPlayer, KilledPawn, DamageType);
	// Reduce our team's live player count.
	ALastimPlayerState* PS = Cast<ALastimPlayerState>(KilledPlayer->PlayerState);
	if (PS)
	{
		if (AliveTeamMemberCount.Num() <= PS->GetTeamNum())
		{
			AliveTeamMemberCount.AddZeroed(PS->GetTeamNum() + 1 - AliveTeamMemberCount.Num());
		}
		AliveTeamMemberCount[PS->GetTeamNum()]--;
	}
}

void AGameMode_LastTeamStanding::DetermineMatchWinner()
{
	// Same implementation as Team Anarchy.
	ALastimGameState const* const MyGameState = Cast<ALastimGameState>(GameState);
	int32 BestScore = MAX_uint32;
	int32 BestTeam = -1;
	int32 NumBestTeams = 1;

	for (int32 i = 0; i < MyGameState->TeamArray.Num(); i++)
	{
		const int32 TeamScore = MyGameState->TeamArray[i]->GetScore();
		if (BestScore < TeamScore)
		{
			BestScore = TeamScore;
			BestTeam = i;
			NumBestTeams = 1;
		}
		else if (BestScore == TeamScore)
		{
			NumBestTeams++;
		}
	}

	//WinnerTeam = (NumBestTeams == 1) ? BestTeam : NumTeams;
}

void AGameMode_LastTeamStanding::CheckForMatchWinner()
{
	ALastimGameState const* const MyGameState = Cast<ALastimGameState>(GameState);

	int32 AliveTeams = 0;
	for (int32 i = 0; i < MyGameState->TeamArray.Num(); i++)
	{
		const int32 TeamLives = MyGameState->TeamArray[i]->GetScore();
		if (TeamLives > 0)
		{
			AliveTeams++;
		}
	}
	// Finish the game if all but one team is out of lives.
	if (AliveTeams <= 1)
	{
		FinishMatch();
	}
}


