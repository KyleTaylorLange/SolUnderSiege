// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/GameState.h"
#include "LastimPlayerState.h"
#include "LastimGameState.generated.h"

/**
 * LastimGameState
 *   This class holds data relevant to the entire game.
 *   It currently has functions for team-based games, but this might be
 *     better served in a subclass.
 */

 /** ranked PlayerState map, created from the GameState */
typedef TMap<int32, TWeakObjectPtr<ALastimPlayerState> > RankedPlayerMap;

UCLASS()
class LASTIM_API ALastimGameState : public AGameState
{
	GENERATED_BODY()

public:

	/** Number of teams in the game. Maybe move to future TeamState classes? **/
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	int32 TeamCount;

	/* List of TeamState classes. */
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	TArray<class ATeamState*> TeamArray;

	/** Time left for warmup / match. **/
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	int32 RemainingTime;

	/** Score limit required to win. **/
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	int32 ScoreLimit;

	/** Time limit until game ends. **/
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	int32 TimeLimit;

	/** Is timer paused? **/
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	bool bTimerPaused;

	/** Are players forced to respawn? **/
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	bool bForceRespawn;

	/** Array of team colours. Maybe move to future TeamState classes? **/
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	TArray<FLinearColor> TeamColor;

	/** Gets a ranked PlayerState map for a specific team. */
	void GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const;
};
