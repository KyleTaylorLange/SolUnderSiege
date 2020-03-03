// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
//#include "SolAIController.h" //Maybe transfer this to the AI controller itself?
#include "SolPlayerState.h"
#include "SolAIController.h"
#include "SolGameMode.generated.h"

UCLASS(minimalapi)
class ASolGameMode : public AGameMode
{
	GENERATED_BODY()

	virtual void PreInitializeComponents() override;

	/** Select the best spawn point for the player. **/
	//virtual AActor* ChoosePlayerStart(AController* Player) override;

	/** Always pick new random spawn. **/
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	/** Choose a start spot based on our criteria. */
	AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** Creates AIControllers for all bots. **/
	void CreateBotControllers();

	/** Creates a bot. **/
	class ASolAIController* CreateBot(struct FBotProfile InBotProfile);

protected:
	/** Ticks every second. */
	virtual void DefaultTimer();

	/* Called when the timer hits zero. */
	virtual void CheckGameTime();

public:
	ASolGameMode(const FObjectInitializer& ObjectInitializer);

	/** Name for this gametype. **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Game)
	FText DisplayName;

	UFUNCTION(exec)
	void FinishMatch();

	/** Modify damage by game rules. **/
	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	/* Time until players can respawn. */
	UPROPERTY(config)
	float RespawnTime;

	/** If true, automatically respawns players. */
	UPROPERTY(config)
	bool bForceRespawn;

	/** Called when someone dies. Used to score kills/deaths and such. **/
	virtual void OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	/* Broadcasts death to local clients. */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastDeath(class ASolPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ASolPlayerState* KilledPlayerState);
	void BroadcastDeath_Implementation(class ASolPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ASolPlayerState* KilledPlayerState);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	virtual void SpawnInventoryForPawn(APawn* InPawn);

	/** Returns true if the player should consider the target an enemy. */
	virtual bool PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const;

	/* Returns the respawn time for this specific player. */
	virtual float GetRespawnTime(ASolPlayerState* AskingPlayer) const;

	UFUNCTION(exec)
	class ASolAIController* AddBot();

	/** @return GameSession class to use for this game  */
	virtual TSubclassOf<class AGameSession> GetGameSessionClass() const;

protected:

	/** Called before match starts. **/
	virtual void HandleMatchIsWaitingToStart() override;

	/** Starts new match. **/
	virtual void HandleMatchHasStarted() override;

	/* Modifies the map's objective points for the specific game mode. */
	virtual void ProcessObjectivePoint(class AObjectivePoint* InOP);

	/** Determine the winner(s). **/
	virtual void DetermineMatchWinner();

	/** Check if any player or team has met the victory conditions, and FinishMatch() if there are. **/
	virtual void CheckForMatchWinner();

	/** Handle for efficient management of DefaultTimer timer. */
	FTimerHandle TimerHandle_DefaultTimer;

	/** Do bots need creation? **/
	bool bNeedsBotCreation;

	/** Spawn bots for this game **/
	void StartBots();

	/** Initialize bot after creation. */
	virtual void InitBot(class ASolAIController* AIC, struct FBotProfile* InBotProfile);

	/** Check if player can use spawnpoint. */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

	/** Check if player should use spawnpoint. */
	virtual bool IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const;

	/** Scores a kill according to the game's rules. **/
	virtual void ScoreKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	/** Scores a death according to the game's rules. **/
	virtual void ScoreDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	/** Score required to win the match. **/
	UPROPERTY(config)
	int32 ScoreLimit;

	/** Time until match automatically ends. Zero means no time limit. **/
	UPROPERTY(config)
	int32 TimeLimit;

	/** Time left in current match. **/
	int32 RemainingTime;

	/** Maximum amount of bots. **/
	UPROPERTY(config)
	int32 MaxBots;

	/** Score upon killing someone. **/
	UPROPERTY(config)
	int32 KillScore;

	/** Score for team upon killing someone. **/
	UPROPERTY(config)
	int32 KillScoreForTeam;

	/** Score upon death. **/
	UPROPERTY(config)
	int32 DeathScore;

	/** Score for team upon death. **/
	UPROPERTY(config)
	int32 DeathScoreForTeam;

	/** Score upon suicide. **/
	UPROPERTY(config)
	int32 SuicideScore;

	/** Score for team upon suicide. **/
	UPROPERTY(config)
	int32 SuicideScoreForTeam;

	/* List of bot profiles. */
	UPROPERTY(Config)
	TArray<struct FBotProfile> BotProfiles;

	/** The class of AI controller for bots. */
	UPROPERTY(EditAnywhere, noclear, BlueprintReadOnly, Category = Classes)
	TSubclassOf<class ASolAIController> BotAIControllerClass;

	/* List of sidearms that the players randomly start with. */
	UPROPERTY(config)
	TArray<TSubclassOf<class AInventoryItem>> InitialSidearms;

	/* List of primary weapons that the players randomly start with (if bSpawnWithPrimary is true). */
	UPROPERTY(config)
	TArray<TSubclassOf<class AInventoryItem>> InitialPrimaryWeapons;

	/* Should the player start with a random primary weapon?*/
	UPROPERTY(config)
	bool bSpawnWithPrimary;
};




