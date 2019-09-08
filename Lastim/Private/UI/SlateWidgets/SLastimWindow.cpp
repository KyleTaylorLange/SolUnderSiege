// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "STextComboBox.h"
#include "LastimMenuHUD.h"
#include "LastimLocalPlayer.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLastimWindow::Construct(const FArguments& InArgs)
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

	WindowSetup();

	ChildSlot
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(SBorder)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.BorderImage(WhiteBrush)
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		.Padding(1)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Center)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(FLinearColor(0.8f, 0.5f, 0.5f, 1.0f))
				//.Padding(5, 2, 5, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Left)//.FillWidth(1)
					[
						SNew(SBox)
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
							.ColorAndOpacity(FLinearColor::White)
							.ShadowColorAndOpacity(FLinearColor::Black)
							.ShadowOffset(FIntPoint(-1, 1))
							.Text(FText::FromString(FString("Test Text")))
						]
					]
					+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).AutoWidth()
					[
						SNew(SBox)
					]
					+ SHorizontalBox::Slot().VAlign(VAlign_Center).HAlign(HAlign_Right)//.FillWidth(1)
					[
						SNew(SButton)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.OnClicked(this, &SLastimWindow::CloseWindow)
						//.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
						[
							SNew(STextBlock)
							.ColorAndOpacity(FLinearColor::White)
							.ShadowColorAndOpacity(FLinearColor::Black)
							.ShadowOffset(FIntPoint(-1, 1))
							//.Font(LargeText)
							.Text(FText::FromString(FString("X")))
						]
					]
				]
			]
			+ SVerticalBox::Slot().MaxHeight(750.f)
			[
				SNew(SBorder)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
				.Padding(5)
				[
					SNew(SBorder)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.BorderImage(WhiteBrush)
					.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
					.Padding(5)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						[
							ConstructWindow()
						]
					]
				]
			]
		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLastimWindow::WindowSetup(){}

TSharedRef<SWidget> SLastimWindow::ConstructWindow()
{
	TSharedPtr<SVerticalBox> Window = SNew(SVerticalBox);


	return Window.ToSharedRef();
}

FReply SLastimWindow::CloseWindow()
{
	if (MainMenuHUD.IsValid())
	{
		ALastimMenuHUD* TestHUD = MainMenuHUD.Get();
		if (TestHUD)
		{
			TestHUD->CloseWindow();
		}
	}

	return FReply::Handled();
}