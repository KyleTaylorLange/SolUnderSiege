// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimMenuPlayerController.h"
#include "SLastimMainMenuWidget.h"
#include "SLastimWindow.h"
#include "LastimGameInstance.h"
#include "SCreateGameScreenWidget.h"
#include "SOptionsScreenWidget.h"
#include "LastimMenuHUD.h"

#define LOCTEXT_NAMESPACE "Lastim.HUD.Menu"

ALastimMenuHUD::ALastimMenuHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	

}

void ALastimMenuHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!CurrentMenu.IsValid())
	{
		const ALastimMenuPlayerController* PCOwner = Cast<ALastimMenuPlayerController>(PlayerOwner);

		if (PCOwner)
		{
			ULocalPlayer* const MyPlayerOwner = Cast<ULocalPlayer>(PCOwner->Player);
			
			SAssignNew(CurrentMenu, SLastimMainMenuWidget)
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

void ALastimMenuHUD::LaunchGame(FString InString)
{
	//LastMenu = CurrentMenu;

	const ALastimMenuPlayerController* PCOwner = Cast<ALastimMenuPlayerController>(PlayerOwner);

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

void ALastimMenuHUD::Options()
{
	const ALastimMenuPlayerController* PCOwner = Cast<ALastimMenuPlayerController>(PlayerOwner);

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

void ALastimMenuHUD::OpenServerBrowser()
{
	const ALastimMenuPlayerController* PCOwner = Cast<ALastimMenuPlayerController>(PlayerOwner);

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

void ALastimMenuHUD::CloseWindow()
{
	OpenWindow = nullptr;
	OpenCGSW = nullptr;
	OpenOSW = nullptr;
	OpenSBW = nullptr;
}