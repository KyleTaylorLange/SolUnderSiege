// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/GameMode.h"
#include "LastimMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ALastimMenuGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
	
	// Begin GameMode interface
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot) override;
	// End GameMode interface

	/** @return GameSession class to use for this game  */
	virtual TSubclassOf<class AGameSession> GetGameSessionClass() const;
};