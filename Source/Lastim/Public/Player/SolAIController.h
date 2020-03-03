// Copyright Kyle Taylor Lange

#pragma once

#include "AIController.h"
#include "SolAIController.generated.h"


// This should eventually be moved to SolBot.
USTRUCT()
struct FBotProfile
{
	GENERATED_USTRUCT_BODY()
	
	/* Bot's name. */
	UPROPERTY()
	FString BotName;

	/* Bot's gender: false for male, true for female. */
	UPROPERTY()
	bool bIsFemale;

	/* Accuracy rating. */
	UPROPERTY()
	float Accuracy;

	/* Alertness rating. */
	UPROPERTY()
	float Alertness;

	UPROPERTY()
	FLinearColor PrimaryColor;

	UPROPERTY()
	FLinearColor SecondaryColor;

	FBotProfile()
	{
		BotName = FString("Bot");
		bIsFemale = false;
		Accuracy = 0.0f;
		Alertness = 0.0f;
		PrimaryColor = FLinearColor(0.35f, 0.35f, 0.4f);
		SecondaryColor = FLinearColor(0.65f, 0.65f, 0.65f);
	}

	FBotProfile(const FString InName, const bool bInIsFemale)
	{
		BotName = InName;
		bIsFemale = bInIsFemale;
		Accuracy = 0.0f;
		Alertness = 0.0f;
		PrimaryColor = FLinearColor(0.35f, 0.35f, 0.4f);
		SecondaryColor = FLinearColor(0.65f, 0.65f, 0.65f);
		// Sexist Test: make women pink by default just so everyone doesn't have the same colours.
		if (bIsFemale)
		{
			PrimaryColor = FLinearColor(0.45f, 0.4f, 0.4f);
			SecondaryColor = FLinearColor(1.0f, 0.41f, 0.7f);
		}
	}
};

/**
 * LASTIM AI CONTROLLER
 *  The controller for the AI.
 *  Instead of using Unreal's behaviour trees, the AI will be totally controlled by C++ code.
 *  TODO: branch bot-specific functions to the SolBot class so that this can be more generic.
 */
UCLASS()
class LASTIM_API ASolAIController : public AAIController
{
	GENERATED_BODY()

protected:

	/** Current pawn the AI considers its enemy. */
	class APawn* CurrentEnemy;

	// Current weapon pickup the AI wants.
	class APickup* CurrentPickup;

	// The enemy's last known location.
	FVector EnemyLastKnownLocation;

	// The enemy's velocity.
	FVector TargetVelocity;

	FVector IdealAimPoint;

	FRotator IdealAimRotation;

	float MaxTargetPredictionError;

	float TargetPredictionError;

	float MaxAimOffsetError;

	float AimOffsetError;

	/* The direction of the bot's aim offset. */
	FVector2D AimOffsetVector;

	float MaxRecoilCompensationError;

	float RecoilCompensationError;

	// Have we lost the enemy?
	bool bHasLostEnemy;

	// Do we need to get a new task?
	bool bWantsNewTask;

	// Are we in the middle of executing GetNextTask()?
	bool bGettingNewTask;

	// The current task as a name.
	FName CurrentTask;

	/** Simple way to not have AI process done every tick. **/
	float AITickTimer;

	/* Speed at which the bot turns. */
	float RotationSpeed;

	/* The bot's vision (dot product). */
	float PeripheralVision;

	/** Handle for efficient management of Respawn timer */
	FTimerHandle TimerHandle_Respawn;

	FTimerHandle TimerHandle_Refire;

	/* The bot's profile. */
	struct FBotProfile BotProfile;
	
public:

	ASolAIController(const FObjectInitializer& ObjectInitializer);

	//virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;
	virtual void OnPossess(class APawn* InPawn) override;
	virtual void BeginInactiveState() override;

	/** Update direction AI is looking based on FocalPoint */
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) override;

	virtual void Tick(float DeltaTime) override;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	/* Finds the closest weapon pickup and sets it as our target weapon pickup. */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	bool FindClosestPickupWithLOS();

	/* Finds closest enemy we can actually see, and sets them as current target. */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	bool FindClosestEnemyWithLOS(ASolCharacter* ExcludeEnemy = nullptr);

	UFUNCTION(BlueprintCallable, Category = Behavior)
	virtual bool IsEnemy(APawn* InPawn);

	UFUNCTION(BlueprintCallable, Category = Behavior)
	virtual void ScanEnvironment();

	UFUNCTION(BlueprintCallable, Category = Behavior)
	virtual void EquipBestWeapon();

	/* Search for specific game objectives. */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	virtual bool GetNextTask();

	/* Search for game-specific objectives. */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	virtual bool CheckObjective();

	/* Find a map location to go to.
	   Returns true if a destination was found. */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	FVector RoamMap();

	/* Can the bot see this object? */
	virtual bool CanSee(AActor* InActor) const;

	/* Can the bot see this location? */
	virtual bool CanSee(FVector InVector) const;

	/** Can the bot see the enemy? */
	bool HasLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const;
	/** Is the bot's weapon pointing at the enemy? */
	bool HasWeaponLOSToEnemy(AActor* InEnemyActor, const bool bAnyEnemy) const;

	// Set the current enemy.
	void SetEnemy(class APawn* InPawn);

	// Clear the current enemy.
	void ClearEnemy();

	/* Get enemy from Blackboard. */
	class APawn* GetEnemy() const;

	// Set the current enemy.
	void SetPickup(class APickup* InPickup);

	// Clear the current enemy.
	void ClearPickup();

	/* Get enemy from Blackboard. */
	class APickup* GetPickup() const;

	/** Update aim inaccuracy. */
	void UpdateAimError(float DeltaSeconds);

	/* Adjusts aim to hit a specific target. */
	void AdjustWeaponAim(FVector & FocalPoint);

	/** Last time aim error was updated. */
	float LastUpdateAimErrorTime;

	/* Begins firing at enemy. */
	UFUNCTION(BlueprintCallable, Category = Behavior)
	virtual void ShootEnemy();

	void Respawn();

	/* Returns the SolCharacter. */
	virtual ASolCharacter* GetSolPawn();

	/* Sets the bot's profile and adjusts bot's properties to match. */
	virtual void SetBotProfile(struct FBotProfile InProfile);
};
