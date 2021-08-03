// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "MainMenuPlayerController.h"
#include "SMainMenuWidget.h"
#include "SLastimWindow.h"
#include "SolGameInstance.h"
#include "SCreateGameScreenWidget.h"
#include "SOptionsScreenWidget.h"
#include "MainMenuHUD.h"

#define LOCTEXT_NAMESPACE "Lastim.HUD.Menu"

AMainMenuHUD::AMainMenuHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	

}

void AMainMenuHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!CurrentMenu.IsValid())
	{
		const AMainMenuPlayerController* PCOwner = Cast<AMainMenuPlayerController>(PlayerOwner);

		if (PCOwner)
		{
			ULocalPlayer* const MyPlayerOwner = Cast<ULocalPlayer>(PCOwner->Player);
			
			SAssignNew(CurrentMenu, SMainMenuWidget)
			.Cursor(EMouseCursor::Default)
			.PlayerOwner(MyPlayerOwner)
			.MainMenuHUD(this);

			if (CurrentMenu.IsValid())
			{

				GEngine->GameViewport->AddViewportWidgetContent(
					SNew(SWeakWidget)
					.PossiblyNullContent(CurrentMenu.ToSharedRef())
					);

				//MyHUDMenuWidget->ActionButtonsWidget->SetVisibility(EVisibility::Visible);
				//MyHUDMenuWidget->ActionWidgetPosition.BindUObject(this, &AStrategyHUD::GetActionsWidgetPos);
			}
			// Setup the widget to forward focus to when the viewport receives focus.
			TSharedPtr<SViewport> GameViewportWidget = GEngine->GetGameViewportWidget();
			if (GameViewportWidget.IsValid())
			{
				//GameViewportWidget->SetWidgetToFocusOnActivate(CurrentMenu);
			}
		}
	}
	if (CurrentMenu.IsValid())
	{
		CurrentMenu->SetVisibility(EVisibility::Visible);
	}
}

void AMainMenuHUD::LaunchGame(FString InString)
{
	//LastMenu = CurrentMenu;

	const AMainMenuPlayerController* PCOwner = Cast<AMainMenuPlayerController>(PlayerOwner);

	if (PCOwner)
	{
		ULocalPlayer* const MyPlayerOwner = Cast<ULocalPlayer>(PCOwner->Player);

		SAssignNew(OpenCGSW, SCreateGameScreenWidget)
			.Cursor(EMouseCursor::Default)
			.PlayerOwner(MyPlayerOwner)
			.MainMenuHUD(this);

		if (OpenCGSW.IsValid())
		{
			GEngine->GameViewport->AddViewportWidgetContent(
				SNew(SWeakWidget)
				.PossiblyNullContent(OpenCGSW.ToSharedRef())
				);
		}
		// Setup the widget to forward focus to when the viewport receives focus.
		TSharedPtr<SViewport> GameViewportWidget = GEngine->GetGameViewportWidget();
		if (GameViewportWidget.IsValid())
		{
			//GameViewportWidget->SetWidgetToFocusOnActivate(OpenCGSW);
		}
	}
	if (OpenCGSW.IsValid())// && LastMenu.IsValid())
	{
		OpenCGSW->SetVisibility(EVisibility::Visible);
	}
}

void AMainMenuHUD::Options()
{
	const AMainMenuPlayerController* PCOwner = Cast<AMainMenuPlayerController>(PlayerOwner);

	if (PCOwner)
	{
		ULocalPlayer* const MyPlayerOwner = Cast<ULocalPlayer>(PCOwner->Player);

		SAssignNew(OpenOSW, SOptionsScreenWidget)
			.Cursor(EMouseCursor::Default)
			.PlayerOwner(MyPlayerOwner)
			.MainMenuHUD(this);

		if (OpenOSW.IsValid())
		{
			GEngine->GameViewport->AddViewportWidgetContent(
				SNew(SWeakWidget)
				.PossiblyNullContent(OpenOSW.ToSharedRef())
				);
		}
		// Setup the widget to forward focus to when the viewport receives focus.
		TSharedPtr<SViewport> GameViewportWidget = GEngine->GetGameViewportWidget();
		if (GameViewportWidget.IsValid())
		{
			//GameViewportWidget->SetWidgetToFocusOnActivate(OpenOSW);
		}
	}
	if (OpenOSW.IsValid())// && LastMenu.IsValid())
	{
		OpenOSW->SetVisibility(EVisibility::Visible);
	}
}

void AMainMenuHUD::OpenServerBrowser()
{
	const AMainMenuPlayerController* PCOwner = Cast<AMainMenuPlayerController>(PlayerOwner);

	if (PCOwner)
	{
		ULocalPlayer* const MyPlayerOwner = Cast<ULocalPlayer>(PCOwner->Player);

		SAssignNew(OpenSBW, SServerBrowserWidget)
			.Cursor(EMouseCursor::Default)
			.PlayerOwner(MyPlayerOwner)
			.MainMenuHUD(this);

		if (OpenSBW.IsValid())
		{
			GEngine->GameViewport->AddViewportWidgetContent(
				SNew(SWeakWidget)
				.PossiblyNullContent(OpenSBW.ToSharedRef())
				);
		}
		// Setup the widget to forward focus to when the viewport receives focus.
		TSharedPtr<SViewport> GameViewportWidget = GEngine->GetGameViewportWidget();
		if (GameViewportWidget.IsValid())
		{
			//GameViewportWidget->SetWidgetToFocusOnActivate(OpenSBW);
		}
	}
	if (OpenSBW.IsValid())// && LastMenu.IsValid())
	{
		OpenSBW->SetVisibility(EVisibility::Visible);
	}
}

void AMainMenuHUD::CloseWindow()
{
	OpenWindow = nullptr;
	OpenCGSW = nullptr;
	OpenOSW = nullptr;
	OpenSBW = nullptr;
}

#undef LOCTEXT_NAMESPACE
