// Copyright Kyle Taylor Lange

#pragma once
#include "SolHUD.h"
#include "SolPlayerState.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */

DECLARE_DELEGATE_RetVal_OneParam(FText, FOnGetPlayerStateAttribute, ASolPlayerState*);

// Probably get rid of this? If we do totals, it would probably be from TeamState (which would include disconnected players).
namespace SpecialPlayerIndex
{
	const int32 All = -1;
}

struct FColumnData
{
	/** Column name */
	FText Name;

	/** Column color */
	FSlateColor Color;

	/** Stat value getter delegate */
	FOnGetPlayerStateAttribute AttributeGetter;

	/** defaults */
	FColumnData()
	{
		Color = FLinearColor::White;
	}

	FColumnData(FText InName, FSlateColor InColor, FOnGetPlayerStateAttribute InAtrGetter)
		: Name(InName)
		, Color(InColor)
		, AttributeGetter(InAtrGetter)
	{
	}
};

struct FTeamPlayer
{
	/** The team the player belongs to */
	uint8 TeamNum;

	/** The number within that team */
	int32 PlayerId;

	/** defaults */
	FTeamPlayer()
		: TeamNum(0)
		, PlayerId(SpecialPlayerIndex::All) // Probably get rid of this?
	{
	}

	FTeamPlayer(uint8 InTeamNum, int32 InPlayerId)
		: TeamNum(InTeamNum)
		, PlayerId(InPlayerId)
	{
	}

	/** comparator */
	bool operator==(const FTeamPlayer& Other) const
	{
		return (TeamNum == Other.TeamNum && PlayerId == Other.PlayerId);
	}

	/** check to see if we have valid player data */
	bool IsValid() const
	{
		return !(*this == FTeamPlayer());
	}
};

class SOL_API SScoreboardWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SScoreboardWidget) {}

	/*See private declaration of OwnerHUD below.*/
	SLATE_ARGUMENT(TWeakObjectPtr<class ASolHUD>, OwnerHUD)

	SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, PCOwner)

	SLATE_END_ARGS()

	/** update PlayerState maps with every tick when scoreboard is shown */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////Needed for every widget
	/////Builds this widget and any of it's children
	void Construct(const FArguments& InArgs);

private:
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/////Pointer to our parent HUD
	/////To make sure HUD's lifetime is controlled elsewhere, use "weak" ptr.
	/////HUD has "strong" pointer to Widget,
	/////circular ownership would prevent/break self-destruction of hud/widget (cause memory leak).
	TWeakObjectPtr<class ASolHUD> OwnerHUD;

	TWeakObjectPtr<class APlayerController> PCOwner;

protected:

	virtual void AddStatColumns();

	int32 PlayerRowWidth;

	/* The scoreboard's innards, e.g. players and their stats. */
	TSharedPtr<SVerticalBox> ScoreboardData;

	/* The scoreboard's innards, e.g. players and their stats. */
	TSharedPtr<SVerticalBox> ScoreboardGrid;

	/* Builds the scoreboard's grid from scratch. */
	virtual void UpdateScoreboardGrid();

	/* Updates the player states, and rebuilds scoreboard if the player count has changed. */
	virtual void UpdatePlayerStateMaps();

	/* Makes the player rows for a specific team. */
	TSharedRef<SWidget> MakePlayerRows(uint8 TeamNum) const;

	/* Makes an individual player row. */
	TSharedRef<SWidget> MakePlayerRow(const FTeamPlayer& TeamPlayer) const;

	ASolPlayerState* GetSortedPlayerState(const FTeamPlayer& TeamPlayer) const;

	/* Colour used for the scoreboard background. */
	FLinearColor ScoreboardTint;

	/* Get the scoreboard row's background color. */
	FSlateColor GetScoreboardBorderColor(const FTeamPlayer TeamPlayer) const;

	/* Get the color for the player's name. */
	FSlateColor GetPlayerColor(const FTeamPlayer TeamPlayer) const;

	/* Is this TeamPlayer the scoreboard's owner? */
	bool IsOwnerPlayer(const FTeamPlayer& TeamPlayer) const;

	/* Gets the player's name. */
	FText GetPlayerName(const FTeamPlayer TeamPlayer) const;

	/* Gets a specific column stat. */
	FText GetStat(FOnGetPlayerStateAttribute Getter, const FTeamPlayer TeamPlayer) const;

	/* TEMP: Gets the player's score. */
	FText GetPlayerScore(ASolPlayerState* PlayerState) const;

	/* Gets the player's score. */
	int32 GetAttributeValue_Score(ASolPlayerState* PlayerState) const;

	/* Gets the player's kills. */
	int32 GetAttributeValue_Kills(ASolPlayerState* PlayerState) const;

	/* Gets the player's deaths. */
	int32 GetAttributeValue_Deaths(ASolPlayerState* PlayerState) const;

	/* Gets the player's score. */
	FText GetAttributeText_Score(ASolPlayerState* PlayerState) const;

	/* Gets the player's kills. */
	FText GetAttributeText_Kills(ASolPlayerState* PlayerState) const;

	/* Gets the player's deaths. */
	FText GetAttributeText_Deaths(ASolPlayerState* PlayerState) const;

	// Gets the team's name.
	FText GetTeamName(uint8 TeamNum) const;

	// Gets the team's score.
	FText GetTeamScore(uint8 TeamNum) const;
	
	// Gets the header text (generally the game mode's name).
	FText GetHeaderText() const;

	// Gets the text for the victory conditions.
	FText GetVictoryConditionsText() const;

	// Get the text for the timer.
	FText GetTimerText() const;

	/** the Ranked PlayerState map...cleared every frame */
	TArray<RankedPlayerMap> PlayerStateMaps;

	/** player count in each team in the last tick */
	TArray<int32> LastTeamPlayerCount;

	/** Stat columns data */
	TArray<FColumnData> StatColumns;
};