// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolGameState.h"
#include "SolPlayerState.h"
#include "SolPlayerController.h"
#include "TeamState.h"
#include "HUD_LastTeamStanding.h"
#include "GameMode_LastTeamStanding.h"

AGameMode_LastTeamStanding::AGameMode_LastTeamStanding(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_LastTeamStanding", "LastTeamStanding", "Last Team Standing");
	HUDClass = AHUD_LastTeamStanding::StaticClass();

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
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
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
	// TODO: There is a bug where no players can spawn on a team despite there being lives available.
	//       Something must be calculating wrong.
	ASolPlayerState* PS = Cast<ASolPlayerState>(NewPlayer->PlayerState);
	if (PS)
	{
		/* Change this later to GetTeam() */
		int32 MyTeamNum = PS->GetTeamNum();
		ASolGameState* MyGameState = Cast<ASolGameState>(GameState);
		if (MyGameState->TeamArray.Num() > MyTeamNum &&
			AliveTeamMemberCount.Num() > MyTeamNum &&
			MyGameState->TeamArray[MyTeamNum]->GetScore() <= AliveTeamMemberCount[MyTeamNum])
		{
			ASolPlayerController* PC = Cast<ASolPlayerController>(NewPlayer);
			if (PC)
			{
				PC->BeginWaiting();
			}
			return;
		}
	}
	Super::RestartPlayer(NewPlayer);
	// If we got this far, add us to our team's live player count.
	// TODO: Is this the cause of the aforementioned spawn error? 
	//       RestartPlayer may not actually restart the player, yet they get added to the live team count.
	if (PS)
	{
		if (AliveTeamMemberCount.Num() <= PS->GetTeamNum())
		{
			AliveTeamMemberCount.AddZeroed(PS->GetTeamNum() + 1 - AliveTeamMemberCount.Num());
		}
		// This PS->GetPawn() call might fix the error (ensure they actually respawned), but more testing is required.
		if (PS->GetPawn())
		{
			AliveTeamMemberCount[PS->GetTeamNum()]++;
		}
	}
}

void AGameMode_LastTeamStanding::OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::OnPlayerDeath(Killer, KilledPlayer, KilledPawn, DamageType);
	// Reduce our team's live player count.
	ASolPlayerState* PS = Cast<ASolPlayerState>(KilledPlayer->PlayerState);
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
	ASolGameState const* const MyGameState = Cast<ASolGameState>(GameState);
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
	ASolGameState const* const MyGameState = Cast<ASolGameState>(GameState);

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


