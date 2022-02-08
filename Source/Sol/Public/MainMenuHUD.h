// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/HUD.h"
#include "SCreateGameScreenWidget.h"
#include "SOptionsScreenWidget.h"
#include "SServerBrowserWidget.h"
#include "MainMenuHUD.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AMainMenuHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

	TSharedPtr<class SCompoundWidget> CurrentMenu;
	TSharedPtr<class SCompoundWidget> LastMenu;
	TSharedPtr<SLastimWindow> OpenWindow;
	TSharedPtr<SCreateGameScreenWidget> OpenCGSW;
	TSharedPtr<SOptionsScreenWidget> OpenOSW;
	TSharedPtr<SServerBrowserWidget> OpenSBW;

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void LaunchGame(FString InString);

	virtual void Options();

	virtual void OpenServerBrowser();
	
	virtual void CloseWindow();
};
