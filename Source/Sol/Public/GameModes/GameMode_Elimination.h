// Copyright Kyle Taylor Lange

#pragma once

#include "TeamGameMode.h"
#include "GameMode_Elimination.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AGameMode_Elimination : public ATeamGameMode
{
	GENERATED_UCLASS_BODY()

	//AGameMode_Elimination(const FObjectInitializer& ObjectInitializer);

protected:

	FName CurrentRoundState;

	virtual void CheckGameTime() override;

	virtual void PrepareToStartRound();

	virtual void HandleRoundStart();

	virtual void HandleRoundEnd();

	virtual void BeginOvertime();

	virtual void EndOvertime();

	virtual void CheckForMatchWinner() override;

	virtual void CheckForRoundWinner();

	virtual void DetermineRoundWinner();

	virtual void StartMatch() override;

	bool bPlayersCanRestart;

	virtual void GetGameOptions(TArray<struct FGameOption> &OptionsList) override;
	
public:

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/* Time round lasts. */
	int32 RoundTime;

	/* Time before round begins. */
	int32 PreRoundTime;

	/* Time after round ends before we enter a new pre-round. */
	int32 PostRoundTime;

	virtual void OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;

	virtual void RestartPlayer(AController* NewPlayer) override;
};
