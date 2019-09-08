// Copyright Kyle Taylor Lange

#pragma once

#include "LastimGameMode.h"
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
class LASTIM_API AGameMode_LastOneStanding : public ALastimGameMode
{
	GENERATED_BODY()

	// Set player's score (lives) to score limit.
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void InitBot(ALastimAIController* AIC, FBotProfile* InBotProfile) override;

public:

	AGameMode_LastOneStanding(const FObjectInitializer& ObjectInitializer);
	
protected:

	/** Override to always force respawn. */
	virtual void HandleMatchHasStarted() override;

	/** Disallow respawning if we are out of lives.
	    Maybe not the best place to do this? */
	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void DetermineMatchWinner() override;

	virtual void CheckForMatchWinner() override;
};
