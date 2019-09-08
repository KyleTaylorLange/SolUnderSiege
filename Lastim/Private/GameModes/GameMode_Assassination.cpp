// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "PlayerState_Assassination.h"
#include "LastimGameState.h"
#include "GameMode_Assassination.h"

AGameMode_Assassination::AGameMode_Assassination(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Assassination", "Assassination", "Assassination");

	//HUDClass = ALastimHUD::StaticClass();
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
	ALastimGameState const* const MyGameState = CastChecked<ALastimGameState>(GameState);
	if (MyGameState != nullptr)
	{
		for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
		{
			APlayerState_Assassination* A_Player = Cast<APlayerState_Assassination>(MyGameState->PlayerArray[i]);
			if (A_Player != nullptr)
			{
				ChooseNewTargetForPlayer(A_Player);
			}
		}
	}
	
	Super::HandleMatchHasStarted();
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
		for (int32 i = 0; i < KillerPlayerState->AllowedKills.Num(); i++)
		{
			if (VictimPlayerState == KillerPlayerState->AllowedKills[i])
			{
				ScoreForThisKill = AssassinationScore;
				bTestAssignKill = false;
				KillerPlayerState->AllowedKills.Remove(VictimPlayerState);
				break;
			}
		}

		/* Check to see if killer eliminated someone hunting them. */
		for (int32 i = 0; i < VictimPlayerState->AllowedKills.Num(); i++)
		{
			if (KillerPlayerState == VictimPlayerState->AllowedKills[i])
			{
				ScoreForThisKill = HunterKillScore;
				bTestAssignKill = false;
				break;
			}
		}

		if (bTestAssignKill)
		{
			VictimPlayerState->AllowedKills.Add(KillerPlayerState);
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

	ALastimGameState const* const MyGameState = CastChecked<ALastimGameState>(GameState);
	if (VictimPlayerState != nullptr && MyGameState != nullptr)
	{
		for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
		{
			APlayerState_Assassination* A_PS = Cast<APlayerState_Assassination>(MyGameState->PlayerArray[i]);
			if (A_PS != nullptr)
			{
				int32 NumRemoved = A_PS->AllowedKills.Remove(VictimPlayerState);
				if (NumRemoved >= 1)
				{
					ChooseNewTargetForPlayer(A_PS);
				}
			}
		}
	}
}

bool AGameMode_Assassination::PlayersAreEnemies(ALastimPlayerState* AskingPlayer, ALastimPlayerState* TargetPlayer) const
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

void AGameMode_Assassination::ChooseNewTargetForPlayer(APlayerState_Assassination* InPlayer)
{
	ALastimGameState const* const MyGameState = CastChecked<ALastimGameState>(GameState);
	if (MyGameState != nullptr && InPlayer != nullptr)
	{
		TArray<APlayerState_Assassination*> PlayerList;
		for (int32 i = 0; i < MyGameState->PlayerArray.Num(); i++)
		{
			APlayerState_Assassination* TestPlayer = Cast<APlayerState_Assassination>(MyGameState->PlayerArray[i]);
			if (TestPlayer != nullptr && TestPlayer != InPlayer && InPlayer->AllowedKills.Find(TestPlayer) == INDEX_NONE)
			{
				PlayerList.Add(TestPlayer);
			}
		}
		/* For now, just choose a random player. */
		InPlayer->AllowedKills.Add(PlayerList[FMath::RandRange(0, PlayerList.Num() - 1)]);
	}
}