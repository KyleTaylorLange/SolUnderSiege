// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/PlayerState.h"
#include "LastimPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ALastimPlayerState : public APlayerState
{
	GENERATED_BODY()

	/**
	 * Set the team
	 *
	 * @param	InController	The controller to initialize state with
	 */
	virtual void ClientInitialize(class AController* InController) override;

public:

	ALastimPlayerState(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	/** Get number of kills. **/
	UFUNCTION(BlueprintCallable, Category = PlayerState)
	int32 GetKills() const;

	/** Get number of deaths. **/
	UFUNCTION(BlueprintCallable, Category = PlayerState)
	int32 GetDeaths() const;

	/** Get number of points. **/
	UFUNCTION(BlueprintCallable, Category = PlayerState)
	int32 GetSuicides() const;

	/** Get number of points. **/
	UFUNCTION(BlueprintCallable, Category = PlayerState)
	float GetScore() const;

	/** Get player's team. **/
	UFUNCTION(BlueprintCallable, Category = PlayerState)
	int32 GetTeamNum() const;

	UFUNCTION(BlueprintCallable, Category = PlayerState)
	class ATeamState* GetTeam() const;

	UFUNCTION(BlueprintCallable, Category = PlayerState)
	FLinearColor GetPrimaryColor() const;

	UFUNCTION(BlueprintCallable, Category = PlayerState)
	FLinearColor GetSecondaryColor() const;

	/** Add to (or subtract from) the player's score. **/
	void AddScore(int32 InScore);

	/** Player killed someone. **/
	void AddKill(ALastimPlayerState* Victim);

	/** Player died. **/
	void AddDeath(ALastimPlayerState* KilledBy);

	/** Player killed themself. **/
	void AddSuicide();

	/* Player killed a teammate. */
	void AddTeamkill(ALastimPlayerState* Victim);

	/** Sets player's team. **/
	void SetTeamNum(int32 NewTeamNumber);

	/** Sets player's team. **/
	void SetTeam(class ATeamState* NewTeam);

	void SetPrimaryColor(FLinearColor NewColor);

	void SetSecondaryColor(FLinearColor NewColor);

	/* Time in seconds until player can respawn. */
	UPROPERTY(Transient, Replicated, BlueprintReadWrite, Category = PlayerState)
	float RespawnTime;
	
protected:

	/** Kills by this player. **/
	UPROPERTY(Transient, Replicated)
	int32 Kills;

	/** How many times associated player has died. **/
	UPROPERTY(Transient, Replicated)
	int32 Deaths;

	/** How many times associated player has killed themself. **/
	UPROPERTY(Transient, Replicated)
	int32 Suicides;

	/* How many teammates this player has killed. */
	UPROPERTY(Transient, Replicated)
	int32 Teamkills;

	/** The player's team. By default, 255 indicates no team. **/
	UPROPERTY(Transient, Replicated)
	int32 TeamNumber;

	/** Player's team. */
	UPROPERTY(Transient, Replicated)
	class ATeamState* Team;

	/** Update player meshs' team colours to match our new one. **/
	virtual void UpdateTeamColors();

	UPROPERTY(Transient, Replicated)
	FLinearColor PrimaryColor;

	UPROPERTY(Transient, Replicated)
	FLinearColor SecondaryColor;
};
