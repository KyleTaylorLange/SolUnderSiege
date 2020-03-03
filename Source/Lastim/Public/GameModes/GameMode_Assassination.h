// Copyright Kyle Taylor Lange

#pragma once

#include "SolGameMode.h"
#include "GameMode_Assassination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AGameMode_Assassination : public ASolGameMode
{
	GENERATED_BODY()

public:

	AGameMode_Assassination(const FObjectInitializer& ObjectInitializer);
	
	/* Tests whether two players are enemies: for assassination, this means one player is a designated target for the other. */
	virtual bool PlayersAreEnemies(ASolPlayerState* AskingPlayer, ASolPlayerState* TargetPlayer) const override;

protected:
	
	int32 AssassinationScore;

	int32 HunterKillScore;

	int32 InnocentKillScore;

	/* Overridden to score kills against designated targets, hunters, and innocents differently. */
	virtual void ScoreKill(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;

	virtual void ScoreDeath(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType) override;

	/* Assign targets to a specific player. */
	virtual void ChooseNewTargetForPlayer(class APlayerState_Assassination* InPlayer);

	/* Overridden to choose an initial target. */
	virtual void HandleMatchHasStarted() override;
};
