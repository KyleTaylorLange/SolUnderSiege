// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "LastimHUD.h"
#include "LastimGameInstance.h"
#include "SInGameMenuWidget.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "Menu"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInGameMenuWidget::Construct(const FArguments& InArgs)
{
	OwnerHUD = InArgs._OwnerHUD;
	PlayerOwner = InArgs._PlayerOwner;
	
	FSlateColor ScoreboardTint = FLinearColor(0.0f, 0.0f, 0.0f, 0.25f);
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FSlateFontInfo LargeText = FSlateFontInfo("Veranda", 24);
	const FSlateFontInfo MediumText = FSlateFontInfo("Veranda", 18);
	const FSlateFontInfo SmallText = FSlateFontInfo("Veranda", 12);

	/** Widgets to look into:
	* SCheckBox - check box.
	* SEditableTextBox - for player name?
	* SSlider - slides between 0 and 1?
	* SComboBox - drop down box?
	**/

	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(SBorder)
		//.Visibility(EVisibility::Visible)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
		[
			SNew(SBorder)
			.Padding(FMargin(15.f))
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(WhiteBrush)
			.BorderBackgroundColor(ScoreboardTint)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.OnClicked(this, &SInGameMenuWidget::GoToMainMenu)
					//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(LargeText)
						.Text(LOCTEXT("MainMenu", "Main Menu"))
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.OnClicked(this, &SInGameMenuWidget::Quit)
					//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
					SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(LargeText)
						.Text(LOCTEXT("Quit", "Quit"))
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SInGameMenuWidget::GoToMainMenu()
{
	ULastimGameInstance* const GameInstance = Cast<ULastimGameInstance>(PlayerOwner->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->GoToState(LastimGameInstanceState::MainMenu);
	}

	return FReply::Handled();
}

FReply SInGameMenuWidget::Quit()
{
	APlayerController* const PCOwner = PlayerOwner.Get() ? PlayerOwner->PlayerController : nullptr;
	if (PCOwner)
	{
		PCOwner->ConsoleCommand("quit");
	}

	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE