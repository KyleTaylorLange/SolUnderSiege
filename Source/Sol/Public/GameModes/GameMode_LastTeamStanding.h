// Copyright Kyle Taylor Lange

#pragma once

#include "TeamGameMode.h"
#include "GameMode_LastTeamStanding.generated.h"

/**
* LAST TEAM STANDING
*  Your team has a total amount of lives, and the goal is to be the last team with lives.
*  There can be multiple teams. 
*/
UCLASS()
class SOL_API AGameMode_LastTeamStanding : public ATeamGameMode
{
	GENERATED_UCLASS_BODY()

	virtual void OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	virtual void GetGameOptions(TArray<struct FGameOption> &OptionsList) override;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:

	/** The amount of currently alive players each team has. Used to determine if we can respawn. */
	TArray<int32> AliveTeamMemberCount;

	/** Override to always force respawn. */
	virtual void HandleMatchHasStarted() override;
	
	/** Begin team scores (lives) at the score limit. */
	virtual void InitTeamState(class ATeamState* InTeam, struct FTeamProfile InProfile) override;

	/** Disallow respawning if our team is out of lives.
	    Maybe not the best place to do this? */
	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void DetermineMatchWinner_Implementation() override;

	virtual void CheckForMatchWinner_Implementation() override;

	// Class of the battle royale shield each player spawns with.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ABattleRoyaleShield> BattleRoyaleShieldClass;

	// Should we spawn with a battle royale shield?
	bool bUseBattleRoyaleShield;
};

