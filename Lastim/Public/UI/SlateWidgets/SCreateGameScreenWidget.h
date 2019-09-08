// Copyright Kyle Taylor Lange

#pragma once
#include "SlateBasics.h"
#include "AssetData.h"
#include "SLastimWindow.h"

struct FGameOption
{
	/* Name of the option in the menu. */
	FText OptionName;
	
	/* String added to the URL. */
	FString URLString;

	/* Widget connected to this option. 
	   TEST: Changed from SCompoundWidget */
public:
	TSharedPtr<class SEditableTextBox> OptionWidget;

	FGameOption(const FText& InOptionName)
	{
		OptionName = InOptionName;
	}

	FGameOption(const FText& InOptionName, const FString& InURLString)
	{
		OptionName = InOptionName;
		URLString = InURLString;
	}

	void SetURLString(const FString& InURLString)
	{
		URLString = InURLString;
	}

	//void SetWidget(TSharedPtr<class SEditableTextBox> InWidget)
	//{
		//OptionWidget = InWidget;
	//}
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
	TSharedPtr< SComboBox< TSharedPtr<FString> > > GameModeListBox;
	/* Display names of the game modes. These are manually keyed in temporarily. Ideally, we get the name from the game mode class itself. */
	TArray<TSharedPtr<FString>> GameModeListArray;
	/* List of text strings for the actual game mode classes. */
	TArray<TSharedPtr<FString>> GameModeClassNameArray;
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

	TSharedRef<SWidget> GenerateStringListWidget(TSharedPtr<FString> InItem);

	/* Updates map drop-down box. */
	void OnMapSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	/* Updates game mode drop-down box. */
	void OnGameModeSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	
};