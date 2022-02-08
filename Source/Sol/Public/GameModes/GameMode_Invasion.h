// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "SolGameMode.h"
#include "GameMode_Invasion.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AGameMode_Invasion : public ASolGameMode
{
	GENERATED_UCLASS_BODY()

	int32 CurrentWave;
	int32 KilledInvaders;
	int32 MaxInvaders;
	int32 MaxWaves;
	int32 WaveLength;

	// Class for invasion opponent controllers.
	UPROPERTY(EditAnywhere, noclear, BlueprintReadOnly, Category = Classes)
	TSubclassOf<class ASolAIController> InvaderControllerClass;

	virtual ASolAIController* CreateInvader();

	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;

	/** Returns true if the player should consider the target an enemy. */
	virtual bool PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const;

protected:

	FTimerHandle TimerHandle_WaveTimer;

	virtual void StartMatch() override;

	virtual void StartNextWave();
};
