// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "STextComboBox.h"
#include "LastimMenuHUD.h"
#include "LastimLocalPlayer.h"
#include "SLastimMainMenuWidget.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "Menu"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLastimMainMenuWidget::Construct(const FArguments& InArgs)
{
	PlayerOwner = InArgs._PlayerOwner;
	MainMenuHUD = InArgs._MainMenuHUD;
	
	FSlateColor ScoreboardTint = FLinearColor(0.0f, 0.0f, 0.0f, 0.25f);
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FSlateFontInfo TitleText = FSlateFontInfo("Veranda", 60);
	const FSlateFontInfo XLText = FSlateFontInfo("Veranda", 36);
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
				.HAlign(HAlign_Center)
				.AutoHeight()
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::White)
					.ShadowColorAndOpacity(FLinearColor::Black)
					.ShadowOffset(FIntPoint(-1, 1))
					.Font(TitleText)
					.Text(FText::FromString(FString("Lastim")))
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.OnClicked(this, &SLastimMainMenuWidget::LaunchGame)
					//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(LargeText)
						.Text(LOCTEXT("CreateGame", "Create Game"))
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.OnClicked(this, &SLastimMainMenuWidget::OpenServerBrowser)
					//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(LargeText)
						.Text(LOCTEXT("ServerBrowser", "Server Browser"))
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.OnClicked(this, &SLastimMainMenuWidget::Options)
					//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(LargeText)
						.Text(LOCTEXT("Options", "Options"))
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.OnClicked(this, &SLastimMainMenuWidget::Quit)
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

FReply SLastimMainMenuWidget::LaunchGame()
{
	if (MainMenuHUD.IsValid())
	{
		ALastimMenuHUD* TestHUD = MainMenuHUD.Get();
		if (TestHUD)
		{
			FString StartStr = FString("");
			TestHUD->LaunchGame(StartStr);
		}
	}

	return FReply::Handled();
}

FReply SLastimMainMenuWidget::OpenServerBrowser()
{
	if (MainMenuHUD.IsValid())
	{
		ALastimMenuHUD* TestHUD = MainMenuHUD.Get();
		if (TestHUD)
		{
			TestHUD->OpenServerBrowser();
		}
	}

	return FReply::Handled();
}

FReply SLastimMainMenuWidget::Options()
{
	if (MainMenuHUD.IsValid())
	{
		ALastimMenuHUD* TestHUD = MainMenuHUD.Get();
		if (TestHUD)
		{
			TestHUD->Options();
		}
	}

	return FReply::Handled();
}

FReply SLastimMainMenuWidget::Quit()
{
	APlayerController* const PCOwner = PlayerOwner.Get() ? PlayerOwner->PlayerController : nullptr;
	if (PCOwner)
	{
		PCOwner->ConsoleCommand("quit");
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE