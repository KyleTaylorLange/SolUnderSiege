// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolGameState.h"
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
	ASolGameState const* const MyGameState = CastChecked<ASolGameState>(GameState);
	float BestScore = MAX_FLT;
	int32 BestPlayer = -1;
	int32 NumBestPlayers = 0;

	for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
	{
		const float PlayerScore = MyGameState->PlayerArray[i]->GetScore();
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

	// Eventually return a winner or something.
}

void AGameMode_Anarchy::CheckForMatchWinner()
{
	ASolGameState const* const MyGameState = CastChecked<ASolGameState>(GameState);

	for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
	{
		const float PlayerScore = MyGameState->PlayerArray[i]->GetScore();
		if (MyGameState->ScoreLimit <= PlayerScore)
		{
			FinishMatch();
		}
	}
}


