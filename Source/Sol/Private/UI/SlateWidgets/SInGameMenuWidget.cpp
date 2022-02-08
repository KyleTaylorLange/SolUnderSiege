// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SComboBox.h"
#include "SolHUD.h"
#include "SolGameInstance.h"
#include "SOptionsScreenWidget.h"
#include "SInGameMenuWidget.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "Menu"
// Ripped off from the ShooterGame example project, as the font directories seemed to have changed in 4.24.
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( FPaths::ProjectContentDir() / "Slate"/ RelativePath + TEXT(".ttf"), __VA_ARGS__ )

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SInGameMenuWidget::Construct(const FArguments& InArgs)
{
	OwnerHUD = InArgs._OwnerHUD;
	PlayerOwner = InArgs._PlayerOwner;
	OptionsWidget = nullptr;
	OptionsWindow = nullptr;

	
	FSlateColor ScoreboardTint = FLinearColor(0.0f, 0.0f, 0.0f, 0.25f);
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FSlateFontInfo LargeText = FSlateFontInfo(FCoreStyle::GetDefaultFont(), 24);
	const FSlateFontInfo MediumText = FSlateFontInfo(FCoreStyle::GetDefaultFont(), 18);
	const FSlateFontInfo SmallText = FSlateFontInfo(FCoreStyle::GetDefaultFont(), 12);

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
					.OnClicked(this, &SInGameMenuWidget::OpenOptions)
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
	USolGameInstance* const GameInstance = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->GoToState(SolGameInstanceState::MainMenu);
	}

	return FReply::Handled();
}

FReply SInGameMenuWidget::OpenOptions()
{
	SAssignNew(OptionsWidget, SOptionsScreenWidget).Cursor(EMouseCursor::Default).PlayerOwner(PlayerOwner);
	if (OptionsWidget.IsValid())
	{
		GEngine->GameViewport->AddViewportWidgetContent(OptionsWidget.ToSharedRef());
		OptionsWidget->SetVisibility(EVisibility::Visible);
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