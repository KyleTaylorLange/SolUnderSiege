// Copyright Kyle Taylor Lange

#pragma once
#include "SlateBasics.h"
#include "AssetData.h"
#include "SLastimWindow.h"

struct FGameMode
{
	FString DisplayName;

	FString ClassName;

	FGameMode(FString DisplayName, FString ClassName)
	{
		this->DisplayName = DisplayName;
		this->ClassName = ClassName;
	}
};

struct FGameOption
{
	/* Name of the option in the menu. */
	FText OptionName;
	
	/* String added to the URL. */
	FString URLString;

	/* Default value to print in this option's box. */
	FText DefaultValue;

	/* Widget connected to this option. 
	   TEST: Changed from SCompoundWidget */
public:
	TSharedPtr<class SEditableTextBox> OptionWidget;

	FGameOption(const FText& InOptionName)
	{
		OptionName = InOptionName;
	}

	FGameOption(const FText& InOptionName, const FString& InURLString, const FText& InDefaultValue)
	{
		OptionName = InOptionName;
		SetURLString(InURLString);
		SetDefaultValue(InDefaultValue);
	}

	void SetURLString(const FString& InURLString)
	{
		URLString = InURLString;
	}

	void SetDefaultValue(const FText& InDefaultValue)
	{
		DefaultValue = InDefaultValue;
	}
};

/**
 * Menu for setting options and creating new games.
 */
class SCreateGameScreenWidget : public SLastimWindow//, public FGCObject
{
public:

	TArray<FGameOption> GameOptions;

	/* Drop-down box to select map. */
	TSharedPtr< SComboBox< TSharedPtr<FString> > > MapListBox;
	/* List of maps.
	We currently have to manually add these in the .cpp file. */
	TArray<TSharedPtr<FString>> MapListArray;
	// TextBlock to update when option is changed.
	TSharedPtr<STextBlock> SelectedMap;

	/* Drop-down box to select the game mode. */
	TSharedPtr< SComboBox< TSharedPtr<FGameMode> > > GameModeListBox;
	/* Display names of the game modes. These are manually keyed in temporarily. Ideally, we get the name from the game mode class itself. */
	TArray<TSharedPtr<FString>> GameModeListArray;
	/* List of text strings for the actual game mode classes. */
	TArray<TSharedPtr<FString>> GameModeClassNameArray;
	/* List of text strings for the actual game mode classes. */
	TArray<TSharedPtr<FGameMode>> GameModes;
	// TextBlock to update when option is changed.
	TSharedPtr<STextBlock> SelectedGameMode;

	/* Makes the list of options. */
	void CreateGameOptions();

	/* Creates the list of game options. */
	TSharedRef<SWidget> MakeGameOptionsList();

	/* Makes an individual game option box. */
	TSharedRef<SWidget> MakeGameOptionWidget(FGameOption& InOption);

	FReply StartGame();

	FReply StartLANGame();

	virtual void HostGame(bool bIsLAN);

	virtual void WindowSetup() override;

	virtual TSharedRef<SWidget> ConstructWindow() override;

protected:

	// Draw list of game modes.
	TSharedRef<SWidget> GenerateGameModesList(TSharedPtr<FGameMode> InItem);

	TSharedRef<SWidget> GenerateStringListWidget(TSharedPtr<FString> InItem);

	/* Updates map drop-down box. */
	void OnMapSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	/* Updates game mode drop-down box. */
	void OnGameModeSelected(TSharedPtr<FGameMode> NewSelection, ESelectInfo::Type SelectInfo);
	
};