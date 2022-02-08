// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "AI_Invasion.h"
#include "SolCharacter.h"
#include "GameMode_Invasion.h"

AGameMode_Invasion::AGameMode_Invasion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Invasion", "Invasion", "Invasion");

	KillScore = 1;
	KillScoreForTeam = 1;
	DeathScore = 0;
	DeathScoreForTeam = 0;
	SuicideScore = -1;
	SuicideScoreForTeam = -1;
	//TeamkillScore = -1;
	//TeamkillScoreForTeam = -1;
	/** The class of AI controller for bots. */
	InvaderControllerClass = AAI_Invasion::StaticClass();

	bDelayedStart = true;

	KilledInvaders = 0;
	CurrentWave = 0;
	MaxInvaders = 5;
	MaxWaves = 10;

	WaveLength = 60;
}

ASolAIController* AGameMode_Invasion::CreateInvader()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = nullptr;

	UWorld* World = GetWorld();
	const TSubclassOf<ASolAIController> BotControllerClass = BotAIControllerClass;
	ASolAIController* InvC = World->SpawnActor<ASolAIController>(InvaderControllerClass, SpawnInfo);
	if (InvC)
	{
		RestartPlayer(InvC);
	}

	return InvC;
}

void AGameMode_Invasion::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);
	ASolCharacter* LChar = Cast<ASolCharacter>(PlayerPawn);
	if (LChar)
	{
		if (LChar->GetController() && LChar->GetController()->IsA(AAI_Invasion::StaticClass()))
		{
			LChar->PrimaryColorOverride = FLinearColor(0.6f, 0.5f, 0.5f, 0.5f);
			LChar->SecondaryColorOverride = FLinearColor(1.0f, 0.1f, 0.1f, 0.5f);
			float FullHealth = 50.f + (CurrentWave * 12.5f);
			LChar->SetMaxHealth(FullHealth * 1.5f);
			LChar->SetFullHealth(FullHealth);
			LChar->SetHealth(FullHealth);
		}
	}
}

void AGameMode_Invasion::StartMatch()
{
	Super::StartMatch();
	
	// Temporary test to automatically make more waves.
	GetWorldTimerManager().SetTimer(TimerHandle_WaveTimer, this, &AGameMode_Invasion::StartNextWave, 15, false);
	//GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Blue, FString::Printf(TEXT("    ~~GAME START~~")));
}

void AGameMode_Invasion::StartNextWave()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_WaveTimer);
	if (CurrentWave >= MaxWaves)
	{
		FinishMatch();
	}
	else
	{
		int32 NumInvaderAIs = 0;
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AAI_Invasion* Ctrlr = Cast<AAI_Invasion>(Iterator->Get());
			if (Cast<AAI_Invasion>(Iterator->Get()))
			{
				NumInvaderAIs++;
			}
		}
		int32 InvadersToMake = FMath::Max(0, CurrentWave + 2 + GameState->PlayerArray.Num() - NumInvaderAIs);
		for (int32 i = 0; i < InvadersToMake; i++)
		{
			CreateInvader();
		}

		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, FString::Printf(TEXT("    ~~CREATED %d INVADERS~~"), InvadersToMake));

		CurrentWave++;
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Green, FString::Printf(TEXT("    ~~WAVE %d~~"), CurrentWave));
		GetWorldTimerManager().SetTimer(TimerHandle_WaveTimer, this, &AGameMode_Invasion::StartNextWave, WaveLength, false);
	}
}

bool AGameMode_Invasion::PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const
{
	// Assume anything with a player state is a non-invader.
	if (AskingPlayer && TargetPlayer)
	{
		return false;
	}
	return true;
}