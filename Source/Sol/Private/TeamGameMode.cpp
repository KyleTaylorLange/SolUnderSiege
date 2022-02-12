// Copyright Kyle Taylor Lange

#include "Sol.h"
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
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Red Team")), SOL_COLOR_RED));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Blue Team")), SOL_COLOR_BLUE));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Green Team")), SOL_COLOR_GREEN));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Gold Team")), SOL_COLOR_GOLD));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Purple Team")), SOL_COLOR_PURPLE));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Orange Team")), SOL_COLOR_ORANGE));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Cyan Team")), SOL_COLOR_CYAN));
	TeamProfiles.Add(FTeamProfile(FText::FromString(FString("Silver Team")), SOL_COLOR_SILVER));
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
	MaxTeams = UGameplayStatics::GetIntOption(Options, "MaxTeams", MaxTeams);
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

		TArray<FTeamProfile> TeamList = ChooseTeamProfiles();
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
				InitTeamState(NewTeam, TeamList[i]);
			}
		}
	}
}

TArray<FTeamProfile> ATeamGameMode::ChooseTeamProfiles()
{
	TArray<FTeamProfile> TeamList = TeamProfiles;
	// COLOR RANDOMIZATION - for fun!
	// Do rarer changes 3/16ths of the time.
	float PctChance = 0.1875f;
	// Swap red or blue for a similar color
	if (TeamCount == 2 && FMath::FRand() <= PctChance)
	{
		if (FMath::RandBool())
		{
			TeamList.Swap(0, FMath::RandBool() ? 3 : 5); // Red for gold or orange
		}
		else
		{
			TeamList.Swap(1, FMath::RandBool() ? 2 : 6); // Blue for green or cyan
		}
	}
	// Swap green for gold half the time
	if ((TeamCount == 3 || TeamCount == 4) && FMath::RandBool())
	{
		TeamList.Swap(2, 3);
	}
	// Swap green or gold for purple
	if (TeamCount == 4 && FMath::FRand() <= PctChance)
	{
		TeamList.Swap(3, 4);
	}
	// Swap orange for cyan 50/50
	if (TeamCount == 6 && FMath::RandBool())
	{
		TeamList.Swap(5, 6);
	}
	// TODO: Remove excess teams.
	return TeamList;
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

			// If each team has the same number of spawn sets, spawn at the team's spawns only.
			if (NumTeamSpawnSets % TeamCount == 0)
			{
				int32 Divisor = NumTeamSpawnSets / TeamCount;
				int32 TeamForSpawn = (TeamStart->SpawnTeam - 1) / Divisor;
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

void ATeamGameMode::DetermineMatchWinner_Implementation()
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

void ATeamGameMode::CheckForMatchWinner_Implementation()
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

void ATeamGameMode::GetGameOptions(TArray<FGameOption> &OptionsList)
{
	Super::GetGameOptions(OptionsList);
	OptionsList.Add(FGameOption(NSLOCTEXT("Sol.HUD.Menu", "TeamCount", "Teams"), FString("MaxTeams"), FText::FromString(FString::FromInt(MaxTeams))));
}
