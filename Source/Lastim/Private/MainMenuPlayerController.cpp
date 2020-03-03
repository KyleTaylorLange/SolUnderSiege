// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "MainMenuPlayerController.h"

AMainMenuPlayerController::AMainMenuPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = APlayerCameraManager::StaticClass();
	SetHidden(false);
	bShowMouseCursor = true;
}

/*
void AStrategyMenuPlayerController::SetupInputComponent()
{
	// Skip AStrategyPlayerController::SetupInputComponent
	APlayerController::SetupInputComponent();
}
*/


