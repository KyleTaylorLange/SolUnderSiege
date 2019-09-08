// Copyright Kyle Taylor Lange

#pragma once


#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class LASTIM_API SInGameMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SInGameMenuWidget)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<class ALastimHUD>, OwnerHUD)

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)

	SLATE_END_ARGS()

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////Needed for every widget
	/////Builds this widget and any of it's children
	void Construct(const FArguments& InArgs);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////Pointer to our parent HUD
	/////To make sure HUD's lifetime is controlled elsewhere, use "weak" ptr.
	/////HUD has "strong" pointer to Widget,
	/////circular ownership would prevent/break self-destruction of hud/widget (cause memory leak).
	TWeakObjectPtr<class ALastimHUD> OwnerHUD;

	/* Pointer to the local player. */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

protected:

	FReply GoToMainMenu();

	FReply Quit();

};
