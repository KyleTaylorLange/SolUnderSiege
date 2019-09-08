// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "Lastim.h"
#include "LastimHUD.h"
#include "LastimPlayerController.h"
#include "LastimAIController.h"
#include "LastimBot.h"
#include "LastimPlayerState.h"
#include "TeamState.h" // Necessary to add score to teams for base implementation.
#include "LastimPlayerStart.h"
#include "LastimSpectatorPawn.h"
#include "LastimGameState.h"
#include "LastimCharacter.h"
#include "LastimGameSession.h"
#include "LastimGameMode.h"

ALastimGameMode::ALastimGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("LastimGameMode", "BaseGameMode", "Unknown Game Mode");
	
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/MyCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	HUDClass = ALastimHUD::StaticClass();
	PlayerControllerClass = ALastimPlayerController::StaticClass();
	BotAIControllerClass = ALastimBot::StaticClass();
	PlayerStateClass = ALastimPlayerState::StaticClass();
	SpectatorClass = ALastimSpectatorPawn::StaticClass();
	GameStateClass = ALastimGameState::StaticClass();
	bNeedsBotCreation = true;

	KillScore = 1;
	DeathScore = 0;
	SuicideScore = -1;

	KillScoreForTeam = 1;
	DeathScoreForTeam = 0;
	SuicideScoreForTeam = -1;

	RespawnTime = 2.5f;

	/* These should go in an INI file later. For now, they go here. */
	/* Male names */
	BotProfiles.Add(FBotProfile(FString("Caol"), false));
	BotProfiles.Last().Accuracy = 0.25f;
	BotProfiles.Last().Alertness = 0.25f;
	BotProfiles.Add(FBotProfile(FString("Joseph"), false));
	BotProfiles.Last().Accuracy = 0.25f;
	BotProfiles.Last().Alertness = 0.25f;
	BotProfiles.Last().SecondaryColor = FLinearColor(0.75f, 0.1f, 0.1f);
	BotProfiles.Add(FBotProfile(FString("Shaun"), false));
	BotProfiles.Last().Accuracy = 0.25f;
	BotProfiles.Last().Alertness = 0.25f;
	BotProfiles.Last().SecondaryColor = FLinearColor(0.1f, 0.75f, 0.1f);
	BotProfiles.Add(FBotProfile(FString("Hugo"), false));
	BotProfiles.Add(FBotProfile(FString("Felix"), false));
	BotProfiles.Add(FBotProfile(FString("Oliver"), false));
	BotProfiles.Last().Accuracy = -0.5f;
	BotProfiles.Last().Alertness = 0.5f;
	BotProfiles.Add(FBotProfile(FString("Oscar"), false));
	BotProfiles.Last().Accuracy = -0.4f;
	BotProfiles.Last().Alertness = 0.4f;
	BotProfiles.Add(FBotProfile(FString("Pavel"), false));
	BotProfiles.Last().Accuracy = -0.3f;
	BotProfiles.Last().Alertness = 0.3f;
	BotProfiles.Add(FBotProfile(FString("Sergey"), false));
	BotProfiles.Last().Accuracy = -0.2f;
	BotProfiles.Last().Alertness = 0.2f;
	BotProfiles.Add(FBotProfile(FString("Marcus"), false));
	BotProfiles.Last().Accuracy = -0.1f;
	BotProfiles.Last().Alertness = 0.1f;
	BotProfiles.Add(FBotProfile(FString("Clark"), false));
	BotProfiles.Add(FBotProfile(FString("Curt"), false));
	BotProfiles.Add(FBotProfile(FString("Samuel"), false));
	BotProfiles.Add(FBotProfile(FString("Spike"), false));
	BotProfiles.Add(FBotProfile(FString("Daisuke"), false));
	BotProfiles.Last().Accuracy = 0.5f;
	BotProfiles.Last().Alertness = 1.0f;
	BotProfiles.Last().PrimaryColor = FLinearColor(0.15f, 0.15f, 0.2f);
	BotProfiles.Last().SecondaryColor = FLinearColor(0.25f, 0.1f, 0.35f);
	BotProfiles.Add(FBotProfile(FString("Hiro"), false));
	BotProfiles.Add(FBotProfile(FString("Tenton"), false));
	BotProfiles.Add(FBotProfile(FString("Pablo"), false));
	BotProfiles.Last().Accuracy = -1.0f;
	BotProfiles.Last().Alertness = -1.0f;
	/* Female names */
	BotProfiles.Add(FBotProfile(FString("Zalia"), true));
	BotProfiles.Last().Accuracy = 1.0f;
	BotProfiles.Last().Alertness = 1.0f;
	BotProfiles.Last().PrimaryColor = FLinearColor(0.1f, 0.1f, 0.1f);
	BotProfiles.Last().SecondaryColor = FLinearColor(0.1f, 0.2f, 0.2f);
	BotProfiles.Add(FBotProfile(FString("Cora"), true));
	BotProfiles.Add(FBotProfile(FString("Aurora"), true));
	BotProfiles.Add(FBotProfile(FString("Sara"), true));
	BotProfiles.Add(FBotProfile(FString("Cassie"), true));
	BotProfiles.Add(FBotProfile(FString("Olivia"), true));
	BotProfiles.Last().Accuracy = 0.5f;
	BotProfiles.Last().Alertness = -0.5f;
	BotProfiles.Add(FBotProfile(FString("Teresa"), true));
	BotProfiles.Last().Accuracy = 0.4f;
	BotProfiles.Last().Alertness = -0.4f;
	BotProfiles.Add(FBotProfile(FString("Victoria"), true));
	BotProfiles.Last().Accuracy = 0.3f;
	BotProfiles.Last().Alertness = -0.3f;
	BotProfiles.Add(FBotProfile(FString("Suki"), true));
	BotProfiles.Last().Accuracy = 0.2f;
	BotProfiles.Last().Alertness = -0.2f;
	BotProfiles.Add(FBotProfile(FString("Mika"), true));
	BotProfiles.Last().Accuracy = 0.1f;
	BotProfiles.Last().Alertness = -0.1f;
}

void ALastimGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &ALastimGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ALastimGameMode::DefaultTimer()
{
	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GameState);
	/* If bForceRespawn is true, spawn players if possible. */
	if (GetMatchState() == MatchState::InProgress && MyGameState && MyGameState->bForceRespawn)
	{
		for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AController* Controller = It->Get();
			if (Controller->IsInState(NAME_Inactive))
			{
				ALastimPlayerState* PS = Cast<ALastimPlayerState>(Controller->PlayerState);
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

void ALastimGameMode::CheckGameTime()
{
	if (GetMatchState() == MatchState::WaitingPostMatch)
	{
		RestartGame();
	}
	else if (GetMatchState() == MatchState::InProgress)
	{
		FinishMatch();
		/**
		// Send end round events
		ALastimGameState* const MyGameState = Cast<ALastimGameState>(GameState);

		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
		ALastimPlayerController* PlayerController = Cast<ALastimPlayerController>(*It);

		if (PlayerController && MyGameState)
		{
		ALastimPlayerState* PlayerState = Cast<ALastimPlayerState>((*It)->PlayerState);
		const bool bIsWinner = IsWinner(PlayerState);

		PlayerController->ClientSendRoundEndEvent(bIsWinner, MyGameState->ElapsedTime);
		}
		}
		**/
	}
	else if (GetMatchState() == MatchState::WaitingToStart)
	{
		StartMatch();
	}
}

void ALastimGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 MaxBotsOptionValue = UGameplayStatics::GetIntOption(Options, "MaxBots"/*GetBotsCountOptionName()*/, MaxBots);
	MaxBots = MaxBotsOptionValue;
	const int32 ScoreLimitOptionValue = UGameplayStatics::GetIntOption(Options, "ScoreLimit"/*GetBotsCountOptionName()*/, ScoreLimit);
	ScoreLimit = ScoreLimitOptionValue;
	const int32 TimeLimitOptionValue = UGameplayStatics::GetIntOption(Options, "TimeLimit"/*GetBotsCountOptionName()*/, TimeLimit);
	TimeLimit = TimeLimitOptionValue;
	Super::InitGame(MapName, Options, ErrorMessage);
}

/** I don't think having a Bot version of the pawn is a good idea, but it's something to consider.
UClass* AShooterGameMode::GetDefaultPawnClassForController(AController* InController)
{
	if (Cast<AShooterAIController>(InController))
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController(InController);
}
**/

void ALastimGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	SpawnInventoryForPawn(PlayerPawn);
}

void ALastimGameMode::SpawnInventoryForPawn(APawn* InPawn)
{
	/* Spawn starting inventory. */
	ALastimCharacter* LCharacter = Cast<ALastimCharacter>(InPawn);
	if (LCharacter != NULL)
	{
		TArray<TSubclassOf<class ALastimInventory>> InventoryList;

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

bool ALastimGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

void ALastimGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	for (TActorIterator<class ALastimObjectivePoint> It(GetWorld()); It; ++It)
	{
		class ALastimObjectivePoint* OP = *It;
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
		// start warmup if needed
		ALastimGameState* const MyGameState = Cast<ALastimGameState>(GameState);
		if (MyGameState && MyGameState->RemainingTime == 0)
		{
			const bool bWantsMatchWarmup = true; //!GetWorld()->IsPlayInEditor();
			if (bWantsMatchWarmup && /*WarmupTime*/ 15 > 0)
			{
				MyGameState->RemainingTime = 15; //WarmupTime;
			}
			else
			{
				MyGameState->RemainingTime = 0.0f;
			}
		}
	}
	// Set this here so players know the score/time limit before the game starts.
	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->ScoreLimit = ScoreLimit;
		MyGameState->TimeLimit = TimeLimit;
		MyGameState->bForceRespawn = bForceRespawn;
	}
}

void ALastimGameMode::HandleMatchHasStarted()
{
	bNeedsBotCreation = true;
	Super::HandleMatchHasStarted();
	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GameState);
	MyGameState->RemainingTime = MyGameState->TimeLimit;
	StartBots();

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		ALastimPlayerController* PC = Cast<ALastimPlayerController>(*It);
		if (PC)
		{
			//PC->ClientGameStarted();
		}
	}
}

void ALastimGameMode::FinishMatch()
{
	ALastimGameState* const MyGameState = Cast<ALastimGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			ALastimPlayerState* PlayerState = Cast<ALastimPlayerState>((*It)->PlayerState);
			//const bool bIsWinner = IsWinner(PlayerState);

			//(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// lock all pawns
		// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
		// turning these back on.
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = 10; //TimeBetweenMatches;
	}
}

void ALastimGameMode::ProcessObjectivePoint(class ALastimObjectivePoint* InOP)
{
	// Nothing in the default implementation.
}

/** Currently verbatim from ShooterGame. */
AActor* ALastimGameMode::ChoosePlayerStart_Implementation(AController* Player)
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

bool ALastimGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	// Base implementation: only allow LastimPlayerStarts.
	ALastimPlayerStart* LastimSpawnPoint = Cast<ALastimPlayerStart>(SpawnPoint);
	if (LastimSpawnPoint)
	{
		return true;
	}
	return false;
}

bool ALastimGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Cast<ACharacter>((*DefaultPawnClass)->GetDefaultObject<ACharacter>());

	// Don't spawn on other players.
	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
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

void ALastimGameMode::DetermineMatchWinner()
{
	// Empty in base class.
}

void ALastimGameMode::CheckForMatchWinner()
{
	// Empty in base class.
}

//////////////////////////////////////////////////////////////////////////
// Bots
// (Mostly copied from ShooterGame)

void ALastimGameMode::CreateBotControllers()
{
	UWorld* World = GetWorld();
	int32 ExistingBots = 0;
	int32 ExistingPlayers = 0;
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		ALastimAIController* AIC = Cast<ALastimAIController>(*It);
		if (AIC)
		{
			++ExistingBots;
		}
		ALastimPlayerController* PC = Cast<ALastimPlayerController>(*It);
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

ALastimAIController* ALastimGameMode::CreateBot(FBotProfile InBotProfile)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = nullptr;

	UWorld* World = GetWorld();
	const TSubclassOf<ALastimAIController> BotControllerClass = BotAIControllerClass;
	ALastimAIController* AIC = World->SpawnActor<ALastimAIController>(BotControllerClass, SpawnInfo);
	AIC->SetBotProfile(InBotProfile);
	InitBot(AIC, &InBotProfile);

	return AIC;
}

void ALastimGameMode::InitBot(ALastimAIController* AIC, FBotProfile* InBotProfile)
{
	if (AIC)
	{
		if (AIC->PlayerState)
		{
			AIC->PlayerState->SetPlayerName(InBotProfile->BotName);

			ALastimPlayerState* SolPS = Cast<ALastimPlayerState>(AIC->PlayerState);
			if (SolPS)
			{
				SolPS->SetPrimaryColor(InBotProfile->PrimaryColor);
				SolPS->SetSecondaryColor(InBotProfile->SecondaryColor);
			}
		}
	}
}

ALastimAIController* ALastimGameMode::AddBot()
{
	FBotProfile NewBotProfile = BotProfiles[FMath::RandRange(0, BotProfiles.Num() - 1)];
	ALastimAIController* AIC = CreateBot(NewBotProfile);
	if (AIC)
	{
		RestartPlayer(AIC);
	}
	return AIC;
}

void ALastimGameMode::StartBots()
{
	UWorld* World = GetWorld();
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{
		ALastimAIController* AIC = Cast<ALastimAIController>(*It);
		if (AIC)
		{
			RestartPlayer(AIC);
		}
	}
}

void ALastimGameMode::OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ALastimPlayerState* KillerPlayerState = Killer ? Cast<ALastimPlayerState>(Killer->PlayerState) : NULL;
	ALastimPlayerState* VictimPlayerState = KilledPlayer ? Cast<ALastimPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		ScoreKill(Killer, KilledPlayer, KilledPawn, DamageType);
	}

	if (VictimPlayerState)
	{
		ScoreDeath(Killer, KilledPlayer, KilledPawn, DamageType);
	}
}

void ALastimGameMode::ScoreKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ALastimPlayerState* KillerPlayerState = Killer ? Cast<ALastimPlayerState>(Killer->PlayerState) : NULL;
	ALastimPlayerState* VictimPlayerState = KilledPlayer ? Cast<ALastimPlayerState>(KilledPlayer->PlayerState) : NULL;

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

void ALastimGameMode::ScoreDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ALastimPlayerState* KillerPlayerState = Killer ? Cast<ALastimPlayerState>(Killer->PlayerState) : NULL;
	ALastimPlayerState* VictimPlayerState = KilledPlayer ? Cast<ALastimPlayerState>(KilledPlayer->PlayerState) : NULL;

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

void ALastimGameMode::BroadcastDeath_Implementation(class ALastimPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ALastimPlayerState* KilledPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// All local players get death messages so they can update their huds.
		ALastimPlayerController* TestPC = Cast<ALastimPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Purple, FString::Printf(TEXT("BroadcastDeath_Implementation")));
		}
	}
}

float ALastimGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Return regular damage by default.
	return Damage;
}

bool ALastimGameMode::PlayersAreEnemies(ALastimPlayerState* AskingPlayer, ALastimPlayerState* TargetPlayer) const
{
	// Return true by default.
	return true;
}

float ALastimGameMode::GetRespawnTime(ALastimPlayerState* Player) const
{
	// Return regular respawn time by default.
	return RespawnTime;
}

TSubclassOf<AGameSession> ALastimGameMode::GetGameSessionClass() const
{
	return ALastimGameSession::StaticClass();
}