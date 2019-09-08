// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimMenuPlayerController.h"
#include "LastimMenuHUD.h"
#include "LastimGameSession.h"
#include "LastimMenuGameMode.h"

ALastimMenuGameMode::ALastimMenuGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//setup our custom PC and HUD
	PlayerControllerClass = ALastimMenuPlayerController::StaticClass();
	HUDClass = ALastimMenuHUD::StaticClass();
	//SpectatorClass = AStrategySpectatorPawn::StaticClass();
}

APawn* ALastimMenuGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, class AActor* StartSpot)
{
	return NULL;
}

TSubclassOf<AGameSession> ALastimMenuGameMode::GetGameSessionClass() const
{
	return ALastimGameSession::StaticClass();
}

