// Copyright Kyle Taylor Lange

#pragma once

#include "SolGameMode.h"
#include "GameMode_LastOneStanding.generated.h"

/**
* LAST ONE STANDING
*  You have a total amount of lives, and the goal is to be the last person alive.
*  This gametype is bound to be very camp happy, so we can add methods to reduce
*   this tendency in the future; we can gradually lower player health (by a made
*   up wound or illness) that requires medicine or bandaging.
*  The base implementation is easy to code, so we'll just leave it at that.
*/
UCLASS()
class LASTIM_API AGameMode_LastOneStanding : public ASolGameMode
{
	GENERATED_BODY()

	// Set player's score (lives) to score limit.
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void InitBot(ASolAIController* AIC, FBotProfile* InBotProfile) override;

	/* Determines the player's starting lives.
		This will generally be the score limit, but will be less for late joiners. */
	virtual void InitStartingLives(ASolPlayerState* NewPlayer);

public:

	AGameMode_LastOneStanding(const FObjectInitializer& ObjectInitializer);

	virtual void GetGameOptions(TArray<struct FGameOption>& OptionsList) override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:

	/** Override to always force respawn. */
	virtual void HandleMatchHasStarted() override;

	/** Disallow respawning if we are out of lives.
	    Maybe not the best place to do this? */
	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void DetermineMatchWinner() override;

	virtual void CheckForMatchWinner() override;

	// Class of the battle royale shield each player spawns with.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ABattleRoyaleShield> BattleRoyaleShieldClass;

	// Should we spawn with a battle royale shield?
	bool bUseBattleRoyaleShield;
};
