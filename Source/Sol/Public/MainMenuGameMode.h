// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/GameMode.h"
#include "MainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AMainMenuGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()
	
	// Begin GameMode interface
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot) override;
	// End GameMode interface

	/** @return GameSession class to use for this game  */
	virtual TSubclassOf<class AGameSession> GetGameSessionClass() const;
};