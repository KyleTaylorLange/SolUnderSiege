// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "MainMenuPlayerController.h"
#include "MainMenuHUD.h"
#include "SolGameSession.h"
#include "SolGameState.h"
#include "MainMenuGameMode.h"

AMainMenuGameMode::AMainMenuGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//setup our custom PC and HUD
	PlayerControllerClass = AMainMenuPlayerController::StaticClass();
	HUDClass = AMainMenuHUD::StaticClass();
	// Needed to get game mode and map lists.
	GameStateClass = ASolGameState::StaticClass();
}

APawn* AMainMenuGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot)
{
	return NULL;
}

TSubclassOf<AGameSession> AMainMenuGameMode::GetGameSessionClass() const
{
	return ASolGameSession::StaticClass();
}

