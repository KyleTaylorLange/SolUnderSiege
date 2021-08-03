// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "PlayerState_Assassination.h"
#include "SolGameState.h"
#include "HUD_Assassination.h"
#include "GameMode_Assassination.h"

AGameMode_Assassination::AGameMode_Assassination(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Assassination", "Assassination", "Assassination");
	HUDClass = AHUD_Assassination::StaticClass();
	PlayerStateClass = APlayerState_Assassination::StaticClass();

	KillScore = 0;
	DeathScore = 0;
	SuicideScore = -1;

	AssassinationScore = 5;
	HunterKillScore = 3;
	InnocentKillScore = -1;

	bDelayedStart = true;
}

void AGameMode_Assassination::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void AGameMode_Assassination::DefaultTimer()
{
	Super::DefaultTimer();
	ASolGameState const* const MyGameState = CastChecked<ASolGameState>(GameState);
	// Give new targets to anyone who wants one.
	if (GetMatchState() == MatchState::InProgress)
	{
		for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
		{
			APlayerState_Assassination* Player = Cast<APlayerState_Assassination>(MyGameState->PlayerArray[i]);
				if (PlayerWantsTarget(Player))
				{
					ChooseNewTargetForPlayer(Player, nullptr);
				}
		}
	}
}

void AGameMode_Assassination::ScoreKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	APlayerState_Assassination* KillerPlayerState = Killer ? Cast<APlayerState_Assassination>(Killer->PlayerState) : NULL;
	APlayerState_Assassination* VictimPlayerState = KilledPlayer ? Cast<APlayerState_Assassination>(KilledPlayer->PlayerState) : NULL;

	/* Suicides are handled in ScoreDeath, so only deal with actual kills. */
	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		/* Always add a kill, even if it was an innocent. */
		KillerPlayerState->AddKill(VictimPlayerState);

		/* Assume killed player is a non-sanctioned kill. */
		int32 ScoreForThisKill = InnocentKillScore;
		bool bTestAssignKill = true;

		/* Check to see if victim was an allowed kill for the killer. */
		if (KillerPlayerState->AllowedKills.Contains(VictimPlayerState))
		{
			ScoreForThisKill = AssassinationScore;
		}

		/* Check to see if killer eliminated someone hunting them. */
		else if (VictimPlayerState->AllowedKills.Contains(KillerPlayerState))
		{
			ScoreForThisKill = HunterKillScore;
		}

		//KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
		if (ScoreForThisKill != 0)
		{
			KillerPlayerState->AddScore(ScoreForThisKill);
			CheckForMatchWinner();

		}
	}
}

void AGameMode_Assassination::ScoreDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::ScoreDeath(Killer, KilledPlayer, KilledPawn, DamageType);
	APlayerState_Assassination* KillerPlayerState = Killer ? Cast<APlayerState_Assassination>(Killer->PlayerState) : NULL;
	APlayerState_Assassination* VictimPlayerState = KilledPlayer ? Cast<APlayerState_Assassination>(KilledPlayer->PlayerState) : NULL;

	ASolGameState const* const MyGameState = CastChecked<ASolGameState>(GameState);
	if (VictimPlayerState && MyGameState)
	{
		for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
		{
			APlayerState_Assassination* A_PS = Cast<APlayerState_Assassination>(MyGameState->PlayerArray[i]);
			if (A_PS)
			{
				if (A_PS->AllowedKills.Contains(VictimPlayerState))
				{
					A_PS->AllowedKills.Remove(VictimPlayerState);
					ChooseNewTargetForPlayer(A_PS, VictimPlayerState);
				}
			}
		}
	}
}

bool AGameMode_Assassination::PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const
{
	APlayerState_Assassination* A_AskingPlayer = Cast<APlayerState_Assassination>(AskingPlayer);
	APlayerState_Assassination* A_TargetPlayer = Cast<APlayerState_Assassination>(TargetPlayer);
	if (A_AskingPlayer != nullptr && A_TargetPlayer != nullptr)
	{
		for (int32 i = 0; i < A_AskingPlayer->AllowedKills.Num(); i++)
		{
			if (A_AskingPlayer->AllowedKills[i] == A_TargetPlayer)
			{
				return true;
			}
		}
		for (int32 i = 0; i < A_TargetPlayer->AllowedKills.Num(); i++)
		{
			if (A_TargetPlayer->AllowedKills[i] == A_AskingPlayer)
			{
				return true;
			}
		}
	}
	return false;
}

bool AGameMode_Assassination::PlayerWantsTarget(class APlayerState_Assassination* InPlayer)
{
	// Currently: get a new target if we have none and we are alive.
	return InPlayer && InPlayer->AllowedKills.Num() == 0 && InPlayer->GetPawn();
}

void AGameMode_Assassination::ChooseNewTargetForPlayer(APlayerState_Assassination* InPlayer, APlayerState_Assassination* LastTarget)
{
	ASolGameState const* const MyGameState = CastChecked<ASolGameState>(GameState);
	FString DebugName = FString("nobody");
	if (MyGameState && InPlayer)
	{
		int32 MinHunters = MAX_int32;
		TArray<APlayerState_Assassination*> PlayerList;
		for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
		{
			APlayerState_Assassination* TestTarget = Cast<APlayerState_Assassination>(MyGameState->PlayerArray[i]);
			// Only look for living targets that aren't already on our kill list.
			if (TestTarget && TestTarget != InPlayer && TestTarget->GetPawn() && !InPlayer->AllowedKills.Contains(TestTarget))
			{
				int32 HunterCount = 0;
				// TODO: Make something more efficient than this nasty n-squared loop-in-a-loop.
				for (int32 j = 0; j < MyGameState->PlayerArray.Num(); j++)
				{
					APlayerState_Assassination* OtherPlayer = Cast<APlayerState_Assassination>(MyGameState->PlayerArray[j]);
					if (OtherPlayer && OtherPlayer != TestTarget && OtherPlayer->AllowedKills.Contains(TestTarget))
					{
						HunterCount++;
					}
				}
				// TEMP: Try to not pick the last target again.
				HunterCount = TestTarget == LastTarget ? (HunterCount + 2) : HunterCount;
				// TEMP: Try not to have players assigned to kill each other.
				HunterCount = TestTarget->AllowedKills.Contains(InPlayer) ? (HunterCount + 3) : HunterCount;
				if (HunterCount <= MinHunters)
				{
					if (HunterCount < MinHunters)
					{
						MinHunters = HunterCount;
						PlayerList.Empty();
					}
					PlayerList.Add(TestTarget);
				}
			}
		}
		// Choose a random player from our best candidates.
		if (PlayerList.Num() > 0)
		{
			int32 idx = FMath::RandRange(0, PlayerList.Num() - 1);
			InPlayer->AllowedKills.Add(PlayerList[idx]);
			DebugName = PlayerList[idx]->GetPlayerName();
		}
	}
	UE_LOG(LogGameMode, Display, TEXT("AGameMode_Assassination::ChooseNewTargetForPlayer - %s has been assigned to kill %s."),
		*InPlayer->GetPlayerName(),
		*DebugName);
}
