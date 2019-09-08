// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimGameState.h"
#include "GameMode_Anarchy.h"

AGameMode_Anarchy::AGameMode_Anarchy(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Anarchy", "Anarchy", "Anarchy");
	
	KillScore = 1;
	DeathScore = 0;
	SuicideScore = -1;

	bDelayedStart = true;
}

void AGameMode_Anarchy::DetermineMatchWinner()
{
	ALastimGameState const* const MyGameState = CastChecked<ALastimGameState>(GameState);
	float BestScore = MAX_FLT;
	int32 BestPlayer = -1;
	int32 NumBestPlayers = 0;

	for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
	{
		const float PlayerScore = MyGameState->PlayerArray[i]->Score;
		if (BestScore < PlayerScore)
		{
			BestScore = PlayerScore;
			BestPlayer = i;
			NumBestPlayers = 1;
		}
		else if (BestScore == PlayerScore)
		{
			NumBestPlayers++;
		}
	}

	//WinnerPlayerState = (NumBestPlayers == 1) ? Cast<ALastimPlayerState>(MyGameState->PlayerArray[BestPlayer]) : NULL;
}

void AGameMode_Anarchy::CheckForMatchWinner()
{
	ALastimGameState const* const MyGameState = CastChecked<ALastimGameState>(GameState);

	for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
	{
		const float PlayerScore = MyGameState->PlayerArray[i]->Score;
		if (MyGameState->ScoreLimit <= PlayerScore)
		{
			FinishMatch();
		}
	}
}


