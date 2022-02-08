// Copyright Kyle Taylor Lange

#pragma once
#include "Widgets/SCompoundWidget.h"

/**
 * Menu for setting options and creating new games.
 */
class SLastimWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLastimWindow) {}

	/** weak pointer to the parent HUD base */
	SLATE_ARGUMENT(TWeakObjectPtr<class ULocalPlayer>, PlayerOwner)

	SLATE_ARGUMENT(TWeakObjectPtr<class AMainMenuHUD>, MainMenuHUD)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	TWeakObjectPtr<class AMainMenuHUD> MainMenuHUD;

	virtual void WindowSetup();

	virtual TSharedRef<SWidget> ConstructWindow();

	FReply CloseWindow();

protected:
	
};