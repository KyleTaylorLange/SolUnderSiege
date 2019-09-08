// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimGameState.h"
#include "LastimGameMode.h"
#include "TeamState.h"
#include "SScoreboardWidget.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "Scoreboard"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SScoreboardWidget::Construct(const FArguments& InArgs)
{
	OwnerHUD = InArgs._OwnerHUD;
	PCOwner = InArgs._PCOwner;
	
	ScoreboardTint = FLinearColor(0.0f, 0.0f, 0.0f, 0.25f);
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FSlateFontInfo LargeText = FSlateFontInfo("Veranda", 24);
	const FSlateFontInfo MediumText = FSlateFontInfo("Veranda", 18);
	const FSlateFontInfo SmallText = FSlateFontInfo("Veranda", 12);

	AddStatColumns();
	PlayerRowWidth = 300;

	TSharedPtr<SVerticalBox> ScoreboardFrame;
	UpdatePlayerStateMaps();

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
				SAssignNew(ScoreboardFrame, SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SBorder)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(LargeText)
						.Text(this, &SScoreboardWidget::GetHeaderText)
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SBorder)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(MediumText)
						.Text(this, &SScoreboardWidget::GetVictoryConditionsText)
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SAssignNew(ScoreboardGrid, SVerticalBox)
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoHeight()
				[
					SNew(SBorder)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(MediumText)
						.Text(this, &SScoreboardWidget::GetTimerText)
					]
				]
			]
		]
	];

	ScoreboardGrid->AddSlot().AutoHeight()
	[
		SAssignNew(ScoreboardData, SVerticalBox)
	];
	UpdateScoreboardGrid();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SScoreboardWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	UpdatePlayerStateMaps();
}

void SScoreboardWidget::AddStatColumns()
{
	StatColumns.Add(FColumnData(LOCTEXT("ScoreColumn", "Score"),
		FLinearColor::White,
		FOnGetPlayerStateAttribute::CreateSP(this, &SScoreboardWidget::GetAttributeText_Score)));

	StatColumns.Add(FColumnData(LOCTEXT("KillsColumn", "Kills"),
		FLinearColor::White,
		FOnGetPlayerStateAttribute::CreateSP(this, &SScoreboardWidget::GetAttributeText_Kills)));

	StatColumns.Add(FColumnData(LOCTEXT("DeathsColumn", "Deaths"),
		FLinearColor::White,
		FOnGetPlayerStateAttribute::CreateSP(this, &SScoreboardWidget::GetAttributeText_Deaths)));
}

void SScoreboardWidget::UpdateScoreboardGrid()
{
	if (!ScoreboardData.IsValid())
	{
		return;
	}
	ScoreboardData->ClearChildren();

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	ScoreboardData->AddSlot().AutoHeight()
		[
			SNew(SBorder)
			.Padding(1)
		.BorderImage(WhiteBrush)
		];

	for (uint8 TeamNum = 0; TeamNum < PlayerStateMaps.Num(); TeamNum++)
	{
		ScoreboardData->AddSlot().AutoHeight()
		[
			MakePlayerRows(TeamNum)
		];
		ScoreboardData->AddSlot().AutoHeight()
		[
			SNew(SBorder)
			.Padding(1)
			.BorderImage(WhiteBrush)
		];
	}
}

void SScoreboardWidget::UpdatePlayerStateMaps()
{
	if (PCOwner.IsValid())
	{
		ALastimGameState* const GameState = Cast<ALastimGameState>(PCOwner->GetWorld()->GetGameState());
		if (GameState)
		{
			bool bRequiresWidgetUpdate = false;
			const int32 TeamCount = FMath::Max(GameState->TeamCount, 1);
			LastTeamPlayerCount.Reset();
			LastTeamPlayerCount.AddZeroed(PlayerStateMaps.Num());
			for (int32 i = 0; i < PlayerStateMaps.Num(); i++)
			{
				LastTeamPlayerCount[i] = PlayerStateMaps[i].Num();
			}

			PlayerStateMaps.Reset();
			PlayerStateMaps.AddZeroed(TeamCount);

			for (int32 i = 0; i < TeamCount; i++)
			{
				// If there's one team, assume FFA. Team index is 255 in FFA.
				// I forgot I did that. Probably fix this later? Set to -1?
				if (TeamCount == 1)
				{
					GameState->GetRankedMap(255, PlayerStateMaps[i]);
				}
				else
				{
					GameState->GetRankedMap(i, PlayerStateMaps[i]);
				}
				// Requiring an update if LastTeamPlayerCount has no values fixes the scoreboard not building on client until a player joins/leaves the game.
				if (LastTeamPlayerCount.Num() == 0 || (LastTeamPlayerCount.Num() > 0 && PlayerStateMaps[i].Num() != LastTeamPlayerCount[i]))
				{
					bRequiresWidgetUpdate = true;
				}
			}
			if (bRequiresWidgetUpdate)
			{
				UpdateScoreboardGrid();
			}
		}
	}
}

TSharedRef<SWidget> SScoreboardWidget::MakePlayerRows(uint8 TeamNum) const
{
	TSharedPtr<SVerticalBox> PlayerRows = SNew(SVerticalBox);

	// Only make team headers if we are in a team game.
	if (PlayerStateMaps.Num() > 1)
	{
		PlayerRows->AddSlot()//.AutoHeight().VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(1)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(FSlateFontInfo("Veranda", 12))
						.Text(this, &SScoreboardWidget::GetTeamName, TeamNum)
					]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SBox)
					.HAlign(HAlign_Right)
					.WidthOverride(100)
					[
						SNew(STextBlock)
						.ColorAndOpacity(FLinearColor::White)
						.ShadowColorAndOpacity(FLinearColor::Black)
						.ShadowOffset(FIntPoint(-1, 1))
						.Font(FSlateFontInfo("Veranda", 12))
						.Text(this, &SScoreboardWidget::GetTeamScore, TeamNum)
					]
				]
			]
		];
	}

	// Make regular column headers.
	TSharedPtr<SHorizontalBox> HeaderRow = SNew(SHorizontalBox);
	HeaderRow->AddSlot()//.AutoHeight().VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.Padding(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor::Black)
		[
			SNew(SBox)
			.WidthOverride(PlayerRowWidth)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::White)
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(-1, 1))
				.Font(FSlateFontInfo("Veranda", 9))
				.Text(LOCTEXT("PlayerName", "Player Name"))
			]
		]
	];
	for (int32 i = 0; i < StatColumns.Num(); i++)
	{
		HeaderRow->AddSlot()//.AutoHeight().VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(1)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
			[
				SNew(SBox)
				.WidthOverride(75)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::White)
					.ShadowColorAndOpacity(FLinearColor::Black)
					.ShadowOffset(FIntPoint(-1, 1))
					.Font(FSlateFontInfo("Veranda", 9))
					.Text(StatColumns[i].Name)
				]
			]
		];
	}
	PlayerRows->AddSlot().AutoHeight()
	[
		HeaderRow.ToSharedRef()
	];
	// Create each player row.
	for (int32 i = 0; i < PlayerStateMaps[TeamNum].Num(); i++)
	{
		PlayerRows->AddSlot().AutoHeight()
		[
			MakePlayerRow(FTeamPlayer(TeamNum, i))
		];
	}

	return PlayerRows.ToSharedRef();
}

TSharedRef<SWidget> SScoreboardWidget::MakePlayerRow(const FTeamPlayer& TeamPlayer) const
{
	TSharedPtr<SHorizontalBox> PlayerRow; // = SNew(SHorizontalBox);
	TSharedPtr<SBorder> PlayerRowBorder = SNew(SBorder).Padding(1).BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
	[
		SAssignNew(PlayerRow, SHorizontalBox)
	];

	PlayerRow->AddSlot()//.AutoHeight().VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.Padding(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Left)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(this, &SScoreboardWidget::GetScoreboardBorderColor, TeamPlayer)
		[
			SNew(SBox)
			.WidthOverride(PlayerRowWidth)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.ColorAndOpacity(this, &SScoreboardWidget::GetPlayerColor, TeamPlayer)
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FIntPoint(-1, 1))
				.Font(FSlateFontInfo("Veranda", 12))
				.Text(this, &SScoreboardWidget::GetPlayerName, TeamPlayer)
			]
		]
	];

	for (int32 i = 0; i < StatColumns.Num(); i++)
	{
		PlayerRow->AddSlot()//.AutoHeight().VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(1)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(this, &SScoreboardWidget::GetScoreboardBorderColor, TeamPlayer)
			[
				SNew(SBox)
				.WidthOverride(75)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.ColorAndOpacity(this, &SScoreboardWidget::GetPlayerColor, TeamPlayer)
					.ShadowColorAndOpacity(FLinearColor::Black)
					.ShadowOffset(FIntPoint(-1, 1))
					.Font(FSlateFontInfo("Veranda", 12))
					.Text(this, &SScoreboardWidget::GetStat, StatColumns[i].AttributeGetter, TeamPlayer)
				]
			]
		];
	}
	return PlayerRowBorder.ToSharedRef();
}

ALastimPlayerState* SScoreboardWidget::GetSortedPlayerState(const FTeamPlayer& TeamPlayer) const
{
	if (PlayerStateMaps.IsValidIndex(TeamPlayer.TeamNum) && PlayerStateMaps[TeamPlayer.TeamNum].Contains(TeamPlayer.PlayerId))
	{
		return PlayerStateMaps[TeamPlayer.TeamNum].FindRef(TeamPlayer.PlayerId).Get();
	}

	return NULL;
}

FSlateColor SScoreboardWidget::GetScoreboardBorderColor(const FTeamPlayer TeamPlayer) const
{
	FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.25f);

	ALastimPlayerState* PlayerState = GetSortedPlayerState(TeamPlayer);
	if (PlayerState && OwnerHUD.IsValid())
	{
		FLinearColor TeamColour = OwnerHUD->GetTeamColor(PlayerState->GetTeam());
		TeamColour.A = 0.25f;
		BackgroundColor = TeamColour;
	}
	if (IsOwnerPlayer(TeamPlayer))
	{
		BackgroundColor.A = 0.5f;
	}

	return FSlateColor(BackgroundColor);
}

FSlateColor SScoreboardWidget::GetPlayerColor(const FTeamPlayer TeamPlayer) const
{
	// If this is the owner player's row, tint the text color to show ourselves more clearly.
	if (IsOwnerPlayer(TeamPlayer))
	{
		return FSlateColor(FLinearColor::Yellow);
	}

	//const FTextBlockStyle& TextStyle = FShooterStyle::Get().GetWidgetStyle<FTextBlockStyle>("ShooterGame.DefaultScoreboard.Row.StatTextStyle");
	return FSlateColor(FLinearColor::White); // TextStyle.ColorAndOpacity;
}

bool SScoreboardWidget::IsOwnerPlayer(const FTeamPlayer& TeamPlayer) const
{
	return (PCOwner.IsValid() && PCOwner->PlayerState && PCOwner->PlayerState == GetSortedPlayerState(TeamPlayer));
}

FText SScoreboardWidget::GetPlayerName(const FTeamPlayer TeamPlayer) const
{
	const ALastimPlayerState* PlayerState = GetSortedPlayerState(TeamPlayer);
	if (PlayerState)
	{
		return FText::FromString(PlayerState->GetPlayerName()); //(PlayerState->GetShortPlayerName());
	}

	return FText::GetEmpty();
}

FText SScoreboardWidget::GetStat(FOnGetPlayerStateAttribute Getter, const FTeamPlayer TeamPlayer) const
{
	ALastimPlayerState* PlayerState = GetSortedPlayerState(TeamPlayer);
	if (PlayerState)
	{
		return Getter.Execute(PlayerState);
	}

	return FText::GetEmpty();
}

FText SScoreboardWidget::GetPlayerScore(ALastimPlayerState* PlayerState) const
{
	return FText::AsNumber(GetAttributeValue_Score(PlayerState));
}

int32 SScoreboardWidget::GetAttributeValue_Score(ALastimPlayerState* PlayerState) const
{
	return FMath::TruncToInt(PlayerState->Score);
}

int32 SScoreboardWidget::GetAttributeValue_Kills(ALastimPlayerState* PlayerState) const
{
	return PlayerState->GetKills();
}

int32 SScoreboardWidget::GetAttributeValue_Deaths(ALastimPlayerState* PlayerState) const
{
	return PlayerState->GetDeaths();
}

FText SScoreboardWidget::GetAttributeText_Score(ALastimPlayerState* PlayerState) const
{
	if (PlayerState)
	{
		return FText::AsNumber(PlayerState->Score);
	}

	return FText::GetEmpty();
}

FText SScoreboardWidget::GetAttributeText_Kills(ALastimPlayerState* PlayerState) const
{
	if (PlayerState)
	{
		return FText::AsNumber(PlayerState->GetKills());
	}

	return FText::GetEmpty();
}

FText SScoreboardWidget::GetAttributeText_Deaths(ALastimPlayerState* PlayerState) const
{
	if (PlayerState)
	{
		return FText::AsNumber(PlayerState->GetDeaths());
	}

	return FText::GetEmpty();
}

FText SScoreboardWidget::GetTeamName(uint8 TeamNum) const
{
	// Get name from team state.
	ALastimGameState* const GameState = Cast<ALastimGameState>(PCOwner->GetWorld()->GetGameState());
	if (GameState && GameState->TeamArray.IsValidIndex(TeamNum))
	{
		return GameState->TeamArray[TeamNum]->GetTeamName();
	}
	// Otherwise, do this:
	return FText::FromString(FString("Unknown Team"));
}

FText SScoreboardWidget::GetTeamScore(uint8 TeamNum) const
{
	ALastimGameState* const GameState = Cast<ALastimGameState>(PCOwner->GetWorld()->GetGameState());
	if (GameState && GameState->TeamArray.IsValidIndex(TeamNum) && GameState->TeamArray[TeamNum] != nullptr)
	{
		return FText::AsNumber(GameState->TeamArray[TeamNum]->GetScore());
	}
	else
	{
		return FText::FromString("!ERROR!");
	}

	return FText::GetEmpty();
}

FText SScoreboardWidget::GetHeaderText() const
{
	FText OutText;

	OutText = OwnerHUD->GetHeaderText();

	return OutText;
}

FText SScoreboardWidget::GetVictoryConditionsText() const
{
	FText OutText;

	OutText = OwnerHUD->GetVictoryConditionsText();

	return OutText;
}

FText SScoreboardWidget::GetTimerText() const
{
	FText OutText;

	OutText = FText::FromString(OwnerHUD->GetTimerString());

	return OutText;
}

#undef LOCTEXT_NAMESPACE