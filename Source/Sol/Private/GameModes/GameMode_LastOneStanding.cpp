// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolGameState.h"
#include "SolPlayerState.h"
#include "SolPlayerController.h"
#include "SolBot.h"
#include "HUD_LastOneStanding.h"
#include "BattleRoyaleShield.h"
#include "GameMode_LastOneStanding.h"

AGameMode_LastOneStanding::AGameMode_LastOneStanding(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_LastOneStanding", "LastOneStanding", "Last One Standing");
	HUDClass = AHUD_LastOneStanding::StaticClass();
	BattleRoyaleShieldClass = ABattleRoyaleShield::StaticClass();

	KillScore = 0;
	DeathScore = -1;
	SuicideScore = -1;
	bUseBattleRoyaleShield = true;

	bDelayedStart = true;
}

void AGameMode_LastOneStanding::HandleMatchHasStarted()
{
	// Always force respawn despite server option.
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->bForceRespawn = true;
	}
	Super::HandleMatchHasStarted();
}

void AGameMode_LastOneStanding::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	// Set the player's score (lives) to the score limit.
	// If a player joins late, give them two fewer lives than the player with the fewest lives.
	if (CastChecked<ASolPlayerState>(NewPlayer->PlayerState))
	{
		InitStartingLives(CastChecked<ASolPlayerState>(NewPlayer->PlayerState));
	}
}

void AGameMode_LastOneStanding::InitBot(ASolAIController* AIC, FBotProfile* InBotProfile)
{
	Super::InitBot(AIC, InBotProfile);
	if (AIC && CastChecked<ASolPlayerState>(AIC->PlayerState))
	{
		InitStartingLives(CastChecked<ASolPlayerState>(AIC->PlayerState));
	}
}

void AGameMode_LastOneStanding::InitStartingLives(ASolPlayerState* NewPlayer)
{
	int32 StartingLives = ScoreLimit;
	if (GetMatchState() == MatchState::InProgress)
	{
		for (int32 i = 0; i < GetGameState<ASolGameState>()->PlayerArray.Num(); i++)
		{
			if (GetGameState<ASolGameState>()->PlayerArray[i] != NewPlayer)
			{
				// Always start with two less lives than the lowest player: 
				//   one in case the lowest player dies next tick, plus one extra to ensure late joiners are not in a better position than others.
				StartingLives = FMath::Clamp(StartingLives, 0, FMath::TruncToInt(GetGameState<ASolGameState>()->PlayerArray[i]->GetScore()) - 2);
				if (StartingLives <= 0)
				{
					break;
				}
			}
		}
	}
	NewPlayer->SetScore(FMath::Max(0.0f, (float)StartingLives));
}

void AGameMode_LastOneStanding::RestartPlayer(AController* NewPlayer)
{
	// Return if we are out of lives, and go into spectator mode. Maybe not the best implementation?
	if (NewPlayer->PlayerState)
	{
		if (NewPlayer->PlayerState->GetScore() <= 0)
		{
			ASolPlayerController* LPC = Cast<ASolPlayerController>(NewPlayer);
			if (LPC)
			{
				LPC->BeginWaiting();
			}
			return;
		}
	}
	Super::RestartPlayer(NewPlayer);
}

void AGameMode_LastOneStanding::DetermineMatchWinner()
{
	// Identical to Anarchy's implementation.
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

	//WinnerPlayerState = (NumBestPlayers == 1) ? Cast<ASolPlayerState>(MyGameState->PlayerArray[BestPlayer]) : NULL;
}

void AGameMode_LastOneStanding::CheckForMatchWinner()
{
	ASolGameState const* const MyGameState = CastChecked<ASolGameState>(GameState);

	int32 AlivePlayers = 0;
	for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
	{
		const float Lives = MyGameState->PlayerArray[i]->GetScore();
		if (Lives > 0)
		{
			AlivePlayers++;
			// No point in checking if multiple players are alive.
			if (AlivePlayers > 1)
			{
				return;
			}
		}
	}
	// Finish the game if there is one (or no) players with lives.
	if (AlivePlayers <= 1)
	{
		FinishMatch();
	}
}

void AGameMode_LastOneStanding::GetGameOptions(TArray<FGameOption> &OptionsList)
{
	Super::GetGameOptions(OptionsList);
	// Rename score to lives.
	for (int32 i = 0; i < OptionsList.Num(); i++)
	{
		if (OptionsList[i].URLString.Equals(FString("ScoreLimit")))
		{
			OptionsList[i].OptionName = NSLOCTEXT("Sol.HUD.Menu", "Lives", "Lives");
			break;
		}
	}
	// Remove bForceRespawn option.
	for (int32 i = 0; i < OptionsList.Num(); i++)
	{
		if (OptionsList[i].URLString.Equals(FString("bForceRespawn")))
		{
			OptionsList.RemoveAt(i);
			break;
		}
	}
}

void AGameMode_LastOneStanding::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	if (bUseBattleRoyaleShield && BattleRoyaleShieldClass)
	{
		StartingInventory.Add(BattleRoyaleShieldClass);
	}
}
