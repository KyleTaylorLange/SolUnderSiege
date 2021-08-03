// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Info.h"
#include "TeamState.generated.h"

/**
 * Contains data about a team like how a PlayerState contains data about a player.
 */
UCLASS()
class LASTIM_API ATeamState : public AInfo
{
	GENERATED_UCLASS_BODY()
	
public:

	/** Get the team's name. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	FText GetTeamName() const;

	/* Sets the name to a specific value. */
	UFUNCTION(BlueprintCallable, Category = TeamState)
	virtual void SetTeamName(FText InName);

	/** Get number of points. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	float GetScore() const;

	/* Adds (or subtracts) score. */
	UFUNCTION(BlueprintCallable, Category = TeamState)
	virtual void AddScore(int32 InScore);
	
	/** Get number of kills. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	int32 GetKills() const;

	/* Adds a kill. */
	UFUNCTION(BlueprintCallable, Category = TeamState)
	virtual void AddKill();

	/** Get number of deaths. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	int32 GetDeaths() const;

	/* Adds a death. */
	UFUNCTION(BlueprintCallable, Category = TeamState)
	virtual void AddDeath();

	/** Get number of suicides. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	int32 GetSuicides() const;

	/* Adds a suicide (and a death). */
	UFUNCTION(BlueprintCallable, Category = TeamState)
	virtual void AddSuicide();

	/** Get number of teamkills. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	int32 GetTeamkills() const;

	/* Adds a teamkill. */
	UFUNCTION(BlueprintCallable, Category = TeamState)
	virtual void AddTeamkill();

	/** Get number of teamkills. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	int32 GetTeamIndex() const;

	/** Get number of teamkills. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	void SetTeamIndex(int32 NewIndex);

	/** Get the team's primary colour. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	FLinearColor GetTeamColor() const;

	/** Get the team's primary colour. **/
	UFUNCTION(BlueprintCallable, Category = TeamState)
	void SetTeamColor(FLinearColor NewColor);

protected:

public:
	/* The name of the team. */
	UPROPERTY(Replicated)
	FText TeamName;
protected:

	/* Total score for this team. */
	UPROPERTY(Replicated)
	int32 Score;

	/* Total kills for this team. */
	UPROPERTY(Replicated)
	int32 Kills;

	/* Total deaths for this team. */
	UPROPERTY(Replicated)
	int32 Deaths;

	/* Total suicides for this team. */
	UPROPERTY(Replicated)
	int32 Suicides;

	/* Total teamkills for this team. */
	UPROPERTY(Replicated)
	int32 Teamkills;

	/* This team's index. */
	UPROPERTY(Replicated)
	int32 TeamIndex;

	/* Primary colour for this team. */
	UPROPERTY(Replicated)
	FLinearColor TeamColor;
};
