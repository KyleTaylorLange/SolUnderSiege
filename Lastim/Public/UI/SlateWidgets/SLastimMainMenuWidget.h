// Copyright Kyle Taylor Lange

#pragma once


#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class LASTIM_API SLastimMainMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLastimMainMenuWidget) {}

	/** weak pointer to the parent HUD base */
	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)

	SLATE_ARGUMENT(TWeakObjectPtr<class ALastimMenuHUD>, MainMenuHUD)

	SLATE_END_ARGS()

	/** update PlayerState maps with every tick when scoreboard is shown */
	//virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:

	void Construct(const FArguments& InArgs);

	/* Pointer to the LocalPlayer, hopefully a LastimLocalPlayer. */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////Pointer to our parent HUD
	/////To make sure HUD's lifetime is controlled elsewhere, use "weak" ptr.
	/////HUD has "strong" pointer to Widget,
	/////circular ownership would prevent/break self-destruction of hud/widget (cause memory leak).
	TWeakObjectPtr<class ALastimMenuHUD> MainMenuHUD;

protected:

	FReply LaunchGame();

	FReply Options();

	FReply OpenServerBrowser();

	FReply Quit();
};
