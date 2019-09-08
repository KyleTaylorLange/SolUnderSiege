// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimStyle.h"
#include "LastimScoreboardWidget.h"

void SLastimScoreboardWidget::Construct(const FArguments& InArgs)
{
	/**
	ScoreboardStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterScoreboardStyle>("DefaultShooterScoreboardStyle");

	PCOwner = InArgs._PCOwner;
	ScoreboardTint = FLinearColor(0.0f, 0.0f, 0.0f, 0.4f);
	FLinearColor ScoreboardBlack = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	**/
	FLinearColor ScoreboardTestColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.4f);
	FLinearColor ScoreboardClear = FLinearColor(1.0f, 1.0f, 1.0f, 0.1f);
	/**
	ScoreBoxWidth = 140.0f;
	ScoreCountUpTime = 2.0f;

	ScoreboardStartTime = FPlatformTime::Seconds();
	MatchState = InArgs._MatchState.Get();

	UpdatePlayerStateMaps();

	Columns.Add(FColumnData(LOCTEXT("KillsColumn", "Kills").ToString(),
		ScoreboardStyle->KillStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Kills)));

	Columns.Add(FColumnData(LOCTEXT("DeathsColumn", "Deaths").ToString(),
	ScoreboardStyle->DeathStatColor,
	FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Deaths)));

	Columns.Add(FColumnData(LOCTEXT("ScoreColumn", "Score").ToString(),
		ScoreboardStyle->ScoreStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Score)));

	TSharedPtr<SHorizontalBox> HeaderCols;

	const TSharedRef<SVerticalBox> ScoreboardGrid = SNew(SVerticalBox)
		// HEADER ROW			
		+ SVerticalBox::Slot().AutoHeight()
		[
			//Padding in the header row (invisible) for speaker icon
			SAssignNew(HeaderCols, SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(NORM_PADDING + FMargin(2, 0, 0, 0)).AutoWidth()
			[
				SNew(SImage)
				.Image(FShooterStyle::Get().GetBrush("ShooterGame.Speaker"))
				.Visibility(EVisibility::Hidden)
			]

			//Player Name autosized column
			+ SHorizontalBox::Slot().Padding(NORM_PADDING)
				[
					SNew(SBorder)
					.Padding(NORM_PADDING)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.BorderImage(&ScoreboardStyle->ItemBorderBrush)
					.BorderBackgroundColor(ScoreboardTint)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PlayerNameColumn", "Player Name").ToString())
						.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
					]
				]
		];

	for (uint8 ColIdx = 0; ColIdx < Columns.Num(); ColIdx++)
	{
		//Header constant sized columns
		HeaderCols->AddSlot().VAlign(VAlign_Center).HAlign(HAlign_Center).AutoWidth().Padding(NORM_PADDING)
			[
				SNew(SBorder)
				.Padding(NORM_PADDING)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.BorderImage(&ScoreboardStyle->ItemBorderBrush)
				.BorderBackgroundColor(ScoreboardBlack)
				[
					SNew(SBox)
					.WidthOverride(ScoreBoxWidth)
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(Columns[ColIdx].Name)
							.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
							.ColorAndOpacity(Columns[ColIdx].Color)
						]
					]
				]
			];
	}

	ScoreboardGrid->AddSlot().AutoHeight()
		[
			SAssignNew(ScoreboardData, SVerticalBox)
		];
	UpdateScoreboardGrid();
	**/

	/** we don't even use this anymore, so let's remove it since it gives a warning.
	FString TestText = FString::Printf(TEXT("TEST TEXT!"));
	TSharedRef<SHorizontalBox> TestBox = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(TestText)
					//.TextStyle(FLastimStyle::Get(), "Lastim.TestStyle")
				]
			];

	SBorder::Construct(
		SBorder::FArguments()
		//.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		.BorderBackgroundColor(ScoreboardTestColor)
		[
			TestBox //ScoreboardGrid
		]
	);
	*/
}


