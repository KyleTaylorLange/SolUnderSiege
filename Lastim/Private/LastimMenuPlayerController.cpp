// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimMenuPlayerController.h"

ALastimMenuPlayerController::ALastimMenuPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = APlayerCameraManager::StaticClass();
	bHidden = false;
	bShowMouseCursor = true;
}

/*
void AStrategyMenuPlayerController::SetupInputComponent()
{
	// Skip AStrategyPlayerController::SetupInputComponent
	APlayerController::SetupInputComponent();
}
*/


