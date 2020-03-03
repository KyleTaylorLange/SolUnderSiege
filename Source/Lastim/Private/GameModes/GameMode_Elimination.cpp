// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolGameState.h"
#include "SolCharacter.h"
#include "TeamState.h"
#include "GameMode_Elimination.h"


//////////////////////////////////////////////////////////////////////////
// ASolCharacter

AGameMode_Elimination::AGameMode_Elimination(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Elimination", "Elimination", "Elimination");
	
	KillScore = 1;
	KillScoreForTeam = 0;
	DeathScore = 0;
	DeathScoreForTeam = 0;
	SuicideScore = -1;
	SuicideScoreForTeam = 0;
	TeamkillScore = -1;
	TeamkillScoreForTeam = 0;
	
	PreRoundTime = 5;
	RoundTime = 150;
	PostRoundTime = 5;

	bPlayersCanRestart = true;
	bDelayedStart = true;
}

namespace RoundState
{
	const FName InPreRound = FName(TEXT("InPreRound"));
	const FName InProgress = FName(TEXT("InProgress"));
	const FName InPostRound = FName(TEXT("InPostRound"));
	const FName InOvertime = FName(TEXT("InOvertime"));
}

void AGameMode_Elimination::CheckGameTime()
{
	if (GetMatchState() == MatchState::InProgress)
	{
		if (CurrentRoundState == RoundState::InProgress)
		{
			HandleRoundEnd();
		}
		else if (CurrentRoundState == RoundState::InPreRound)
		{
			HandleRoundStart();
		}
		else if (CurrentRoundState == RoundState::InPostRound)
		{
			PrepareToStartRound();
		}
	}
	else
	{
		Super::CheckGameTime();
	}
}

void AGameMode_Elimination::StartMatch()
{
	Super::StartMatch();
	PrepareToStartRound();
}

void AGameMode_Elimination::PrepareToStartRound()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("PrepareToStartRound()")));

	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	if (IsMatchInProgress())
	{
		CurrentRoundState = RoundState::InPreRound; //SetMatchState(MatchState::InPreRound); //EndMatch();

		// Destroy all existing pawns.
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AController* Ctrlr = Iterator->Get();
			if ((Ctrlr->GetPawn() != NULL) /*&& PlayerCanRestart(PlayerController)*/)
			{
				// Should probably be moved to a function in SolCharacter.
				ASolCharacter* OldPawn = Cast<ASolCharacter>(Ctrlr->GetPawn());
				Ctrlr->UnPossess();
				if (OldPawn)
				{
					OldPawn->DestroyInventory();
					OldPawn->DetachFromControllerPendingDestroy();
					OldPawn->TurnOff();
					OldPawn->SetActorHiddenInGame(true);
					OldPawn->SetLifeSpan(1.0f);
				}
			}
		}

		// Restart players
		bPlayersCanRestart = true;
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AController* Ctrlr = Iterator->Get();
			if ((Ctrlr->GetPawn() == NULL) /*&& PlayerCanRestart(PlayerController)*/)
			{
				RestartPlayer(Ctrlr);
			}
			if (Ctrlr->GetPawn())
			{
				/* TODO: Disable movement until round starts. */
				ASolCharacter* LPawn = Cast<ASolCharacter>(Ctrlr->GetPawn());
				if (LPawn)
				{
					LPawn->SetWeaponFiringAllowed(false);
				}
			}
		}

		MyGameState->RemainingTime = PreRoundTime;
	}
}

void AGameMode_Elimination::HandleRoundStart()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("HandleRoundStart()")));
	
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	if (IsMatchInProgress())
	{
		CurrentRoundState = RoundState::InProgress; //SetMatchState(MatchState::InProgress); //EndMatch();

		// Restart players (if they somehow died before the round began)
		bPlayersCanRestart = true;
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AController* Ctrlr = Iterator->Get();
			if ((Ctrlr->GetPawn() == NULL) /*&& PlayerCanRestart(PlayerController)*/)
			{
				RestartPlayer(Ctrlr);
			}
			if (Ctrlr->GetPawn())
			{
				/* TODO: Disable movement until round starts. */
				ASolCharacter* SPawn = Cast<ASolCharacter>(Ctrlr->GetPawn());
				if (SPawn)
				{
					SPawn->SetWeaponFiringAllowed(true);
				}
			}
		}
		bPlayersCanRestart = false;

		MyGameState->RemainingTime = RoundTime;
	}
}

void AGameMode_Elimination::HandleRoundEnd()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("HandleRoundEnd()")));
	
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	if (IsMatchInProgress())
	{
		CurrentRoundState = RoundState::InPostRound;
		DetermineRoundWinner();

		MyGameState->RemainingTime = PostRoundTime;
	}
}

void AGameMode_Elimination::BeginOvertime()
{
	// Do me later
}

void AGameMode_Elimination::EndOvertime()
{
	// Do me later
}

void AGameMode_Elimination::OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::OnPlayerDeath(Killer, KilledPlayer, KilledPawn, DamageType);
	CheckForRoundWinner();
}

void AGameMode_Elimination::CheckForRoundWinner()
{
	ASolGameState const* const MyGameState = Cast<ASolGameState>(GameState);
	TArray<ATeamState*> AliveTeams;

	if (MyGameState)
	{
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			ASolCharacter* Char = Cast<ASolCharacter>(*It);
			if (Char && Char->IsAlive())
			{
				ASolPlayerState* PS = Cast<ASolPlayerState>(Char->GetPlayerState());
				if (PS && PS->GetTeam())
				{
					AliveTeams.AddUnique(PS->GetTeam());
				}
			}
		}
		if (AliveTeams.Num() <= 1)
		{
			HandleRoundEnd();
		}
	}
}

void AGameMode_Elimination::DetermineRoundWinner()
{
	// Same implementation as Team Anarchy.
	ASolGameState const* const MyGameState = Cast<ASolGameState>(GameState);
	TArray<int32> NumAlivePlayers;

	if (MyGameState)
	{
		NumAlivePlayers.AddZeroed(MyGameState->TeamArray.Num());

		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			ASolCharacter* Char = Cast<ASolCharacter>(*It);
			if (Char && Char->IsAlive())
			{
				ASolPlayerState* PS = Cast<ASolPlayerState>(Char->GetPlayerState());
				if (PS && PS->GetTeam())
				{
					NumAlivePlayers[PS->GetTeamNum()]++;
				}
			}
		}

		ATeamState* BestTeam = nullptr;
		int32 BestTeamPlayerCount = 0;

		for (int32 i = 0; i < NumAlivePlayers.Num(); i++)
		{
			if (NumAlivePlayers[i] > BestTeamPlayerCount)
			{
				BestTeam = MyGameState->TeamArray[i];
				BestTeamPlayerCount = NumAlivePlayers[i];
			}
			/* No winner for ties. */
			else if (NumAlivePlayers[i] == BestTeamPlayerCount)
			{
				BestTeam = nullptr;
			}
		}
		if (BestTeam)
		{
			BestTeam->AddScore(1);
			CheckForMatchWinner();
		}
	}
}

void AGameMode_Elimination::CheckForMatchWinner()
{
	ASolGameState const* const MyGameState = Cast<ASolGameState>(GameState);

	for (int32 i = 0; i < MyGameState->TeamArray.Num(); i++)
	{
		const int32 TeamScore = MyGameState->TeamArray[i]->GetScore();
		if (TeamScore >= ScoreLimit)
		{
			FinishMatch();
		}
	}
}

void AGameMode_Elimination::RestartPlayer(AController* NewPlayer)
{
	if (!bPlayersCanRestart)
	{
		return;
	}
	Super::RestartPlayer(NewPlayer);
}