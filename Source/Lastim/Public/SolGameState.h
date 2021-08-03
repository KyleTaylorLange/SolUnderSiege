// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/GameState.h"
#include "SolPlayerState.h"
#include "SolGameState.generated.h"

/**
 * SolGameState
 *   This class holds data relevant to the entire game.
 *   It currently has functions for team-based games, but this might be
 *     better served in a subclass.
 */

 /** ranked PlayerState map, created from the GameState */
typedef TMap<int32, TWeakObjectPtr<ASolPlayerState> > RankedPlayerMap;

UCLASS()
class LASTIM_API ASolGameState : public AGameState
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

	// List of map names. Done as strings until we build a map asset getter.
	UPROPERTY(config)
	TArray<FString> TempMapNames;

	/** Gets a ranked PlayerState map for a specific team. */
	void GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const;

	// Gets list of game modes.
	void GetGameModes(TArray<UClass*>& GameModes) const;

	// Gets a list of maps for a specific game mode.
	void GetMaps(TArray<FAssetData>& Maps, AGameMode* GameMode) const;
};
