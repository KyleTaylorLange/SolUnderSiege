// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolCharacter.h"
#include "SolGameState.h"
#include "SolAIController.h"
#include "SolPlayerState.h"
#include "SolPlayerStart.h"
#include "TeamState.h"
#include "TeamGameMode.h"

ATeamGameMode::ATeamGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("TeamGameMode", "BaseTeamGameMode", "Unknown Team Game Mode");
	NumTeamSpawnSets = 0;
	bPreferTeamSpawns = true;

	TeamStateClass = ATeamState::StaticClass();
	
	MaxTeams = 2;
	TeamkillScore = -1;
	TeamkillScoreForTeam = -1;

	// Default Team Colors
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Red Team")), FLinearColor(357.f, 1.0f, 0.5f).HSVToLinearRGB()));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Blue Team")), FLinearColor(235.f, 1.0f, 0.4f).HSVToLinearRGB()));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Green Team")), FLinearColor(110.f, 1.0f, 0.5f).HSVToLinearRGB()));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Gold Team")), FLinearColor(60.f, 1.0f, 0.5f).HSVToLinearRGB()));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Purple Team")), FLinearColor(290.f, 1.0f, 0.5f).HSVToLinearRGB()));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Orange Team")), FLinearColor(15.f, 1.0f, 0.625f).HSVToLinearRGB()));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Cyan Team")), FLinearColor(160.f, 1.0f, 0.5f).HSVToLinearRGB()));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Silver Team")), FLinearColor(210.f, 0.1f, 0.8f).HSVToLinearRGB()));
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	// Place player on a team before Super (VoIP team based init, findplayerstart, etc)
	ASolPlayerState* NewPlayerState = CastChecked<ASolPlayerState>(NewPlayer->PlayerState);
	ATeamState* BestTeam = ChooseTeam(NewPlayerState);
	NewPlayerState->SetTeam(BestTeam);

	Super::PostLogin(NewPlayer);
}

void ATeamGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	// Go through the player starts and find how many teams the map supports.
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* TestSpawn = *It;
		ASolPlayerStart* TeamPlayerStart = Cast<ASolPlayerStart>(TestSpawn);
		if (TeamPlayerStart)
		{
			if (TeamPlayerStart->SpawnTeam > NumTeamSpawnSets)
			{
				NumTeamSpawnSets = TeamPlayerStart->SpawnTeam;
			}
		}
	}
}

void ATeamGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 MaxTeamsOptionValue = UGameplayStatics::GetIntOption(Options, "MaxTeams"/*GetBotsCountOptionName()*/, MaxTeams);
	MaxTeams = MaxTeamsOptionValue;
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ATeamGameMode::InitGameState()
{
	Super::InitGameState();

	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	if (MyGameState)
	{
		// Currently, clamp teams between 2 and the amount of team profiles.
		TeamCount = FMath::Clamp(MaxTeams, 2, TeamProfiles.Num() );
		MyGameState->TeamCount = TeamCount;

		// TEMP COLOR RANDOMIZATION - for fun!
		// Swap red or blue for a similar color 1/8 of the time
		if (TeamCount == 2 && FMath::FRand() < 0.125f)
		{
			if (FMath::RandBool())
			{
				if (FMath::RandBool())
				{
					TeamProfiles.Swap(0, 5); // Red for orange
				}
				else
				{
					TeamProfiles.Swap(0, 3); // Red for gold
				}
			}
			else
			{
				if (FMath::RandBool())
				{
					TeamProfiles.Swap(1, 2); // Blue for green
				}
				else
				{
					TeamProfiles.Swap(1, 6); // Blue for cyan
				}
			}
		}
		// Swap green for gold 1/2 of the time
		if (TeamCount == 3 && FMath::RandBool())
		{
			TeamProfiles.Swap(2, 3);
		}
		// Swap green or gold for purple 1/8 of the time
		if (TeamCount == 4 && FMath::FRand() < 0.125f)
		{
			if (FMath::RandBool())
			{
				TeamProfiles.Swap(2, 3); // Gold for green
				TeamProfiles.Swap(3, 4); // Green for purple, but make it still last.
			}
			else
			{
				TeamProfiles.Swap(3, 4); // Gold for purple
			}
		}
		// Swap orange for cyan 50/50
		if (TeamCount == 6 && FMath::RandBool())
		{
			TeamProfiles.Swap(5, 6);
		}

		// ACTUAL CODE HERE
		for (int32 i = 0; i < MyGameState->TeamCount; i++)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Instigator = nullptr;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnInfo.OverrideLevel = nullptr;

			UWorld* World = GetWorld();
			const TSubclassOf<ATeamState> NewTeamStateClass = TeamStateClass;
			ATeamState* NewTeam = World->SpawnActor<ATeamState>(NewTeamStateClass, SpawnInfo);
			if (NewTeam)
			{
				MyGameState->TeamArray.Add(NewTeam);
				NewTeam->SetTeamIndex(i);
				InitTeamState(NewTeam, TeamProfiles[i]);
			}
		}
	}
}

void ATeamGameMode::InitTeamState(ATeamState* InTeam, FTeamProfile InProfile)
{
	if (InTeam)
	{
		InTeam->SetTeamColor(InProfile.TeamColor);
		InTeam->SetTeamName(InProfile.TeamName);
	}
}

void ATeamGameMode::InitBot(ASolAIController* AIC, FBotProfile* InBotProfile)
{
	ASolPlayerState* BotPlayerState = CastChecked<ASolPlayerState>(AIC->PlayerState);
	ATeamState* BestTeam = ChooseTeam(BotPlayerState);
	//BotPlayerState->SetTeamNum(TeamNum);
	BotPlayerState->SetTeam(BestTeam);

	Super::InitBot(AIC, InBotProfile);
}

ATeamState* ATeamGameMode::ChooseTeam(ASolPlayerState* ForPlayerState) const
{
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	ATeamState* BestTeam = nullptr;
	if (MyGameState)
	{
		TArray<int32> TeamBalance;
		TeamBalance.AddZeroed(MyGameState->TeamArray.Num());

		// get current team balance
		for (int32 i = 0; i < GameState->PlayerArray.Num(); i++)
		{
			ASolPlayerState const* const TestPlayerState = Cast<ASolPlayerState>(GameState->PlayerArray[i]);
			if (TestPlayerState && TestPlayerState != ForPlayerState && TeamBalance.IsValidIndex(TestPlayerState->GetTeamNum()))
			{
				TeamBalance[TestPlayerState->GetTeamNum()]++;
			}
		}

		// find least populated one
		int32 BestTeamScore = TeamBalance[0];
		for (int32 i = 1; i < TeamBalance.Num(); i++)
		{
			if (BestTeamScore > TeamBalance[i])
			{
				BestTeamScore = TeamBalance[i];
			}
		}

		// there could be more than one...
		TArray<ATeamState*> BestTeams;
		for (int32 i = 0; i < TeamBalance.Num(); i++)
		{
			if (TeamBalance[i] == BestTeamScore)
			{
				BestTeams.Add(MyGameState->TeamArray[i]);
			}
		}

		// get random from best list
		BestTeam = BestTeams[FMath::RandHelper(BestTeams.Num())];
	}
	return BestTeam;
}

bool ATeamGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	// Only perform check if there are two or more team spawn sets.
	if (Player && bPreferTeamSpawns && NumTeamSpawnSets >= 2)
	{
		ASolPlayerStart* TeamStart = Cast<ASolPlayerStart>(SpawnPoint);
		ASolPlayerState* PlayerState = Cast<ASolPlayerState>(Player->PlayerState);

		// Check if spawn is for our team.
		if (PlayerState && TeamStart)
		{
			// Assume spawns aren't balanced, unless one of these below are true.
			bool bSpawnsAreBalanced = false;

			// If same number of spawn sets as teams, spawn at the team's spawns only.
			if (NumTeamSpawnSets == TeamCount)
			{
				if (NumTeamSpawnSets == TeamCount && TeamStart->SpawnTeam != PlayerState->GetTeamNum() + 1)
				{
					return false;
				}
				bSpawnsAreBalanced = true;
			}

			// Use multiple team spawn sets per team if each team has an equal amount of spawn sets.
			else if (NumTeamSpawnSets % TeamCount == 0)
			{
				int TeamForSpawn = (TeamStart->SpawnTeam - 1) % TeamCount;
				if (PlayerState->GetTeamNum() != TeamForSpawn)
				{
					return false;
				}
				bSpawnsAreBalanced = true;
			}

			// If spawns are balanced and the spawn has no team, don't use it.
			if (TeamStart && TeamStart->SpawnTeam == 0 && bSpawnsAreBalanced)
			{
				return false;
			}
		}
	}

	return Super::IsSpawnpointAllowed(SpawnPoint, Player);
}

float ATeamGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Damage;

	ASolCharacter* DamagedPawn = Cast<ASolCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		ASolPlayerState* DamagedPS = Cast<ASolPlayerState>(DamagedPawn->GetPlayerState());
		ASolPlayerState* InstigatorPS = Cast<ASolPlayerState>(EventInstigator->PlayerState);

		// Scale friendly fire.
		if (InstigatorPS->GetTeam() != nullptr && InstigatorPS->GetTeam() == DamagedPS->GetTeam())
		{
			ActualDamage *= FriendlyFireScale;
		}
	}
	return ActualDamage;
}

bool ATeamGameMode::PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const
{
	ATeamState* AskerTeam = AskingPlayer->GetTeam();
	ATeamState* TargetTeam = TargetPlayer->GetTeam();
	if (AskerTeam == TargetTeam)
	{
		return false;
	}
	return true;
}

void ATeamGameMode::OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	Super::OnPlayerDeath(Killer, KilledPlayer, KilledPawn, DamageType);
}

void ATeamGameMode::ScoreKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ASolPlayerState* KillerPlayerState = Killer ? Cast<ASolPlayerState>(Killer->PlayerState) : NULL;
	ASolPlayerState* VictimPlayerState = KilledPlayer ? Cast<ASolPlayerState>(KilledPlayer->PlayerState) : NULL;

	/* Don't score kills for friendly fire. */
	if (KillerPlayerState && VictimPlayerState && (KillerPlayerState != VictimPlayerState))
	{
		if ((KillerPlayerState->GetTeam() != nullptr) && (KillerPlayerState->GetTeam() == VictimPlayerState->GetTeam()))
		{
			bool bCheckForMatchWinner = false;
			KillerPlayerState->AddTeamkill(VictimPlayerState);
			if (TeamkillScore != 0)
			{
				KillerPlayerState->AddScore(TeamkillScore);
				bCheckForMatchWinner = true;
			}
			if (KillerPlayerState->GetTeam())
			{
				KillerPlayerState->GetTeam()->AddTeamkill();
				if (TeamkillScoreForTeam != 0)
				{
					KillerPlayerState->GetTeam()->AddScore(TeamkillScoreForTeam);
					bCheckForMatchWinner = true;
				}
			}
			if (bCheckForMatchWinner)
			{
				CheckForMatchWinner();
			}
			return;
		}
	}
	Super::ScoreKill(Killer, KilledPlayer, KilledPawn, DamageType);
}

void ATeamGameMode::DetermineMatchWinner()
{
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

void ATeamGameMode::CheckForMatchWinner()
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