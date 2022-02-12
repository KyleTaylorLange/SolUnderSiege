// Copyright Kyle Taylor Lange

#pragma once

#include "SolGameMode.h"
#include "TeamGameMode.generated.h"

// Basic info about a team.
USTRUCT()
struct FTeamProfile
{
	GENERATED_USTRUCT_BODY()

	FText TeamName;
	FLinearColor TeamColor;

	FTeamProfile()
	{
		TeamName = FText::FromString("Unknown Team");
		TeamColor = FLinearColor(0.5f, 0.5f, 0.5f);
	}

	FTeamProfile(const FText InName, const FLinearColor InColor)
	{
		TeamName = InName;
		TeamColor = InColor;
	}

};
/**
 * 
 */
UCLASS(Abstract)
class SOL_API ATeamGameMode : public ASolGameMode
{
	GENERATED_BODY()

	/** Setup team changes at player login. **/
	void PostLogin(APlayerController* NewPlayer) override;

	/** Initialize replicated game data. **/
	virtual void InitGameState() override;

	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	ATeamGameMode(const FObjectInitializer& ObjectInitializer);

	/** Set the team amount, and other options. **/
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// Overridden to query team numbers.
	virtual bool PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const override;

	/* Overridden to add teamkills. */
	virtual void OnPlayerDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;

	virtual void GetGameOptions(TArray<FGameOption> &OptionsList) override;

protected:

	/** Figure out NumTeamSpawmSets. **/
	virtual void HandleMatchIsWaitingToStart() override;

	/** The amount of teams the map has spawns for. */
	int32 NumTeamSpawnSets;

	/** Should players spawn at team spawn points? Only works if team distribution is balanced. */
	UPROPERTY(config)
	bool bPreferTeamSpawns;
	
	/** Maximum number of teams in this match. **/
	UPROPERTY(config)
	int32 MaxTeams; // Change to int8 later?

	/**Actual amount of teams in this match. **/
	int32 TeamCount; // Change to int8 later?

	/** The class of AI controller for bots. */
	UPROPERTY(EditAnywhere, noclear, BlueprintReadOnly, Category = Classes)
	TSubclassOf<class ATeamState> TeamStateClass;

	/** Creates a team state. */
	virtual void InitTeamState(class ATeamState* InTeam, struct FTeamProfile InProfile);

	// Select profiles to use for each team.
	virtual TArray<FTeamProfile> ChooseTeamProfiles();

	/** Override to give bots a team. **/
	virtual void InitBot(ASolAIController* AIC, struct FBotProfile* InBotProfile) override;

	/* Overridden to deal with teamkills. */
	virtual void ScoreKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;

	/* Choose the team with the highest score as the winner. */
	virtual void DetermineMatchWinner_Implementation() override;

	/* Check to see if any teams have met the score limit. */
	virtual void CheckForMatchWinner_Implementation() override;

	/** Score upon killing a teammate. **/
	UPROPERTY(config)
	int32 TeamkillScore;

	/** Score for team upon killing a teammate. **/
	UPROPERTY(config)
	int32 TeamkillScoreForTeam;

	/** Picks team with least players in or random when it's equal. */
	class ATeamState* ChooseTeam(class ASolPlayerState* ForPlayerState) const;

	/** Use team spawns. */
	bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const override;

	/** Damage scalar to teammates. **/
	UPROPERTY(config)
	float FriendlyFireScale;

	/** Team colours. **/
	UPROPERTY(config)
	TArray<FTeamProfile> TeamProfiles;

};
