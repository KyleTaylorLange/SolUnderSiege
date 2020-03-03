// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Lastim.h"
#include "SolHUD.h"
#include "SolPlayerController.h"
#include "SolAIController.h"
#include "SolBot.h"
#include "SolPlayerState.h"
#include "TeamState.h" // Necessary to add score to teams for base implementation.
#include "SolPlayerStart.h"
#include "SolSpectatorPawn.h"
#include "SolGameState.h"
#include "SolCharacter.h"
#include "SolGameSession.h"
#include "SolGameMode.h"

ASolGameMode::ASolGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("SolGameMode", "BaseGameMode", "Unknown Game Mode");
	
	// set default pawn class to the Blueprint character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/MyCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	HUDClass = ASolHUD::StaticClass();
	PlayerControllerClass = ASolPlayerController::StaticClass();
	BotAIControllerClass = ASolBot::StaticClass();
	PlayerStateClass = ASolPlayerState::StaticClass();
	SpectatorClass = ASolSpectatorPawn::StaticClass();
	GameStateClass = ASolGameState::StaticClass();
	bNeedsBotCreation = true;

	KillScore = 1;
	DeathScore = 0;
	SuicideScore = -1;

	KillScoreForTeam = 1;
	DeathScoreForTeam = 0;
	SuicideScoreForTeam = -1;

	RespawnTime = 2.5f;
}

void ASolGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &ASolGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ASolGameMode::DefaultTimer()
{
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	/* If bForceRespawn is true, spawn players if possible. */
	if (GetMatchState() == MatchState::InProgress && MyGameState && MyGameState->bForceRespawn)
	{
		for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AController* Controller = It->Get();
			if (Controller->IsInState(NAME_Inactive))
			{
				ASolPlayerState* PS = Cast<ASolPlayerState>(Controller->PlayerState);
				if (PS != NULL && PS->RespawnTime <= 0.0f)
				{
					RestartPlayer(Controller);
				}
			}
		}
	}

	if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused) //&& MyGameState->TimeLimit > 0)
	{
		MyGameState->RemainingTime--;

		if (MyGameState->RemainingTime <= 0)
		{
			CheckGameTime();
		}
	}
}

void ASolGameMode::CheckGameTime()
{
	if (GetMatchState() == MatchState::WaitingPostMatch)
	{
		RestartGame();
	}
	else if (GetMatchState() == MatchState::InProgress)
	{
		FinishMatch();
	}
	else if (GetMatchState() == MatchState::WaitingToStart)
	{
		StartMatch();
	}
}

void ASolGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 MaxBotsOptionValue = UGameplayStatics::GetIntOption(Options, "MaxBots", MaxBots);
	MaxBots = MaxBotsOptionValue;
	const int32 ScoreLimitOptionValue = UGameplayStatics::GetIntOption(Options, "ScoreLimit", ScoreLimit);
	ScoreLimit = ScoreLimitOptionValue;
	const int32 TimeLimitOptionValue = UGameplayStatics::GetIntOption(Options, "TimeLimit", TimeLimit);
	TimeLimit = TimeLimitOptionValue;
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ASolGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	SpawnInventoryForPawn(PlayerPawn);
}

void ASolGameMode::SpawnInventoryForPawn(APawn* InPawn)
{
	/* Spawn starting inventory. */
	ASolCharacter* LCharacter = Cast<ASolCharacter>(InPawn);
	if (LCharacter != NULL)
	{
		TArray<TSubclassOf<class AInventoryItem>> InventoryList;

		// Always spawn a starting sidearm.
		if (InitialSidearms.Num() >= 1)
		{
			const int32 RandIndex = FMath::RandRange(0, InitialSidearms.Num() - 1);
			if (InitialSidearms[RandIndex])
			{
				InventoryList.Add(InitialSidearms[RandIndex]);
			}
			else
			{
				InitialSidearms.RemoveAt(RandIndex);
			}
		}
		// Only spawn with a random primary weapon if the options allow it.
		if (bSpawnWithPrimary && InitialPrimaryWeapons.Num() >= 1)
		{
			const int32 RandIndex = FMath::RandRange(0, InitialPrimaryWeapons.Num() - 1);
			if (InitialPrimaryWeapons[RandIndex])
			{
				InventoryList.Add(InitialPrimaryWeapons[RandIndex]);
			}
			else
			{
				InitialPrimaryWeapons.RemoveAt(RandIndex);
			}
		}
		LCharacter->SpawnInitialInventory(InventoryList);
	}
}

bool ASolGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

void ASolGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	for (TActorIterator<class AObjectivePoint> It(GetWorld()); It; ++It)
	{
		class AObjectivePoint* OP = *It;
		if (OP != nullptr)
		{
			ProcessObjectivePoint(OP);
		}
	}
	if (bNeedsBotCreation)
	{
		CreateBotControllers();
		bNeedsBotCreation = false;
	}
	if (bDelayedStart)
	{
		// start warmup if needed/
		// TODO: Make some type of ready system and make it optional.
		ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
		if (MyGameState && MyGameState->RemainingTime == 0)
		{
			const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
			if (bWantsMatchWarmup && /*WarmupTime*/ 15 > 0)
			{
				MyGameState->RemainingTime = 15; //WarmupTime;
			}
			else
			{
				MyGameState->RemainingTime = 5;
			}
		}
	}
	// Set this here so players know the score/time limit before the game starts.
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->ScoreLimit = ScoreLimit;
		MyGameState->TimeLimit = TimeLimit;
		MyGameState->bForceRespawn = bForceRespawn;
	}
}

void ASolGameMode::HandleMatchHasStarted()
{
	bNeedsBotCreation = true;
	Super::HandleMatchHasStarted();
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	MyGameState->RemainingTime = MyGameState->TimeLimit;
	StartBots();

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		ASolPlayerController* PC = Cast<ASolPlayerController>(*It);
		if (PC)
		{
			//PC->ClientGameStarted();
		}
	}
}

void ASolGameMode::FinishMatch()
{
	ASolGameState* const MyGameState = Cast<ASolGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			ASolPlayerState* PlayerState = Cast<ASolPlayerState>((*It)->PlayerState);
			//const bool bIsWinner = IsWinner(PlayerState);

			//(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// lock all pawns
		// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
		// turning these back on.
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = 15; //TimeBetweenMatches;
	}
}

void ASolGameMode::ProcessObjectivePoint(class AObjectivePoint* InOP)
{
	// Nothing in the default implementation.
}

// Currently verbatim from ShooterGame.
/* TODO: Implement a more complex spawn selection algorithm that rates player starts on some of the following:
   * Whether enemies are nearby/visible.
   * Whether allies are nearby.
   * The time since this spawn was used.
   */
AActor* ASolGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	APlayerStart* BestStart = NULL;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* TestSpawn = *It;
		if (TestSpawn->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			BestStart = TestSpawn;
			break;
		}
		else
		{
			if (IsSpawnpointAllowed(TestSpawn, Player))
			{
				if (IsSpawnpointPreferred(TestSpawn, Player))
				{
					PreferredSpawns.Add(TestSpawn);
				}
				else
				{
					FallbackSpawns.Add(TestSpawn);
				}
			}
		}
	}


	if (BestStart == NULL)
	{
		if (PreferredSpawns.Num() > 0)
		{
			BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
		}
		else if (FallbackSpawns.Num() > 0)
		{
			BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}

bool ASolGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	// Base implementation: only allow SolPlayerStarts.
	ASolPlayerStart* SolSpawnPoint = Cast<ASolPlayerStart>(SpawnPoint);
	if (SolSpawnPoint)
	{
		return true;
	}
	return false;
}

/* TODO: Implement a more complex spawn selection algorithm that rates player starts on some of the following:
   * Whether enemies are nearby/visible.
   * Whether allies are nearby.
   * The time since this spawn was used.
   */
bool ASolGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Cast<ACharacter>((*DefaultPawnClass)->GetDefaultObject<ACharacter>());

	// Don't spawn on other players.
	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn && OtherPawn != MyPawn)
			{
				const float CombinedHeight = (MyPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedRadius = MyPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// check if player start overlaps this pawn
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

void ASolGameMode::DetermineMatchWinner()
{
	// Empty in base class.
}

void ASolGameMode::CheckForMatchWinner()
{
	// Empty in base class.
}

//////////////////////////////////////////////////////////////////////////
// Bots
// (Mostly taken from ShooterGame)

void ASolGameMode::CreateBotControllers()
{
	UWorld* World = GetWorld();
	int32 ExistingBots = 0;
	int32 ExistingPlayers = 0;
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		ASolAIController* AIC = Cast<ASolAIController>(*It);
		if (AIC)
		{
			++ExistingBots;
		}
		ASolPlayerController* PC = Cast<ASolPlayerController>(*It);
		if (PC)
		{
			++ExistingPlayers;
		}
	}

	// Create any necessary AIControllers. Hold off on Pawn creation until pawns are actually necessary or need recreating.	
	for (int32 i = 0; i < MaxBots - ExistingBots - ExistingPlayers; ++i)
	{
		FBotProfile NewBotProfile(FString("Bot"), false);
		if (BotProfiles.Num() > 0)
		{
			int32 BotIndex = FMath::RandRange(0, BotProfiles.Num() - 1);
			NewBotProfile = BotProfiles[BotIndex];
			BotProfiles.RemoveAt(BotIndex, 1);
		}
		CreateBot(NewBotProfile);
	}
}

ASolAIController* ASolGameMode::CreateBot(FBotProfile InBotProfile)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = nullptr;

	UWorld* World = GetWorld();
	const TSubclassOf<ASolAIController> BotControllerClass = BotAIControllerClass;
	ASolAIController* AIC = World->SpawnActor<ASolAIController>(BotControllerClass, SpawnInfo);
	AIC->SetBotProfile(InBotProfile);
	InitBot(AIC, &InBotProfile);

	return AIC;
}

void ASolGameMode::InitBot(ASolAIController* AIC, FBotProfile* InBotProfile)
{
	if (AIC)
	{
		if (AIC->PlayerState)
		{
			AIC->PlayerState->SetPlayerName(InBotProfile->BotName);

			ASolPlayerState* SolPS = Cast<ASolPlayerState>(AIC->PlayerState);
			if (SolPS)
			{
				SolPS->SetPrimaryColor(InBotProfile->PrimaryColor);
				SolPS->SetSecondaryColor(InBotProfile->SecondaryColor);
			}
		}
	}
}

ASolAIController* ASolGameMode::AddBot()
{
	FBotProfile NewBotProfile = BotProfiles[FMath::RandRange(0, BotProfiles.Num() - 1)];
	ASolAIController* AIC = CreateBot(NewBotProfile);
	if (AIC)
	{
		RestartPlayer(AIC);
	}
	return AIC;
}

void ASolGameMode::StartBots()
{
	UWorld* World = GetWorld();
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		ASolAIController* AIC = Cast<ASolAIController>(*It);
		if (AIC)
		{
			RestartPlayer(AIC);
		}
	}
}

void ASolGameMode::OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ASolPlayerState* KillerPlayerState = Killer ? Cast<ASolPlayerState>(Killer->PlayerState) : NULL;
	ASolPlayerState* VictimPlayerState = KilledPlayer ? Cast<ASolPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		ScoreKill(Killer, KilledPlayer, KilledPawn, DamageType);
	}

	if (VictimPlayerState)
	{
		ScoreDeath(Killer, KilledPlayer, KilledPawn, DamageType);
	}
}

void ASolGameMode::ScoreKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ASolPlayerState* KillerPlayerState = Killer ? Cast<ASolPlayerState>(Killer->PlayerState) : NULL;
	ASolPlayerState* VictimPlayerState = KilledPlayer ? Cast<ASolPlayerState>(KilledPlayer->PlayerState) : NULL;

	/* Suicides are handled in ScoreDeath, so only deal with actual kills. */
	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		bool bCheckForMatchWinner = true;
		KillerPlayerState->AddKill(VictimPlayerState);
		//KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
		if (KillScore != 0)
		{
			KillerPlayerState->AddScore(KillScore);
			bCheckForMatchWinner = true;

		}
		if (KillerPlayerState->GetTeam())
		{
			KillerPlayerState->GetTeam()->AddSuicide();
			if (KillScoreForTeam != 0)
			{
				VictimPlayerState->GetTeam()->AddScore(KillScoreForTeam);
				bCheckForMatchWinner = true;
			}
		}
		if (bCheckForMatchWinner)
		{
			CheckForMatchWinner();
		}
	}
}

void ASolGameMode::ScoreDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ASolPlayerState* KillerPlayerState = Killer ? Cast<ASolPlayerState>(Killer->PlayerState) : NULL;
	ASolPlayerState* VictimPlayerState = KilledPlayer ? Cast<ASolPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (VictimPlayerState)
	{
		bool bCheckForMatchWinner = false;
		/** Scoring a suicide also adds a death, so just score suicide. **/
		if (KillerPlayerState && KillerPlayerState == VictimPlayerState)
		{
			VictimPlayerState->AddSuicide();
			BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
			if (SuicideScore != 0)
			{
				VictimPlayerState->AddScore(SuicideScore);
				bCheckForMatchWinner = true;
			}
			if (VictimPlayerState->GetTeam())
			{
				VictimPlayerState->GetTeam()->AddSuicide();
				if (SuicideScoreForTeam != 0)
				{
					VictimPlayerState->GetTeam()->AddScore(SuicideScoreForTeam);
					bCheckForMatchWinner = true;
				}
			}
		}
		else
		{
			VictimPlayerState->AddDeath(KillerPlayerState);
			BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
			if (DeathScore != 0)
			{
				VictimPlayerState->AddScore(DeathScore);
				bCheckForMatchWinner = true;
			}
			if (VictimPlayerState->GetTeam())
			{
				VictimPlayerState->GetTeam()->AddDeath();
				if (DeathScoreForTeam != 0)
				{
					VictimPlayerState->GetTeam()->AddScore(DeathScoreForTeam);
					bCheckForMatchWinner = true;
				}
			}
		}
		if (bCheckForMatchWinner)
		{
			CheckForMatchWinner();
		}
	}
}

void ASolGameMode::BroadcastDeath_Implementation(class ASolPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ASolPlayerState* KilledPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// All local players get death messages so they can update their huds.
		ASolPlayerController* TestPC = Cast<ASolPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
			//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Purple, FString::Printf(TEXT("BroadcastDeath_Implementation")));
		}
	}
}

float ASolGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Return regular damage by default.
	return Damage;
}

bool ASolGameMode::PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const
{
	// Return true by default.
	return true;
}

float ASolGameMode::GetRespawnTime(ASolPlayerState* Player) const
{
	// Return regular respawn time by default.
	return RespawnTime;
}

TSubclassOf<AGameSession> ASolGameMode::GetGameSessionClass() const
{
	return ASolGameSession::StaticClass();
}