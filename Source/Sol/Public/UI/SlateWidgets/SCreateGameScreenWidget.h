// Copyright Kyle Taylor Lange

#pragma once
#include "SlateBasics.h"
#include "AssetData.h"
#include "SLastimWindow.h"
#include "SolGameMode.h"

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
	TSharedPtr<STextBlock> SelectedMapBox;

	/* Drop-down box to select the game mode. */
	TSharedPtr<SComboBox<UClass*>> GameModeListBox;
	/* List of game mode classes found. */
	TArray<UClass*> GameModesList;
	// TextBlock to update when option is changed.
	TSharedPtr<STextBlock> SelectedGameModeBox;

	/* Drop-down box to select the game mode. */
	TSharedPtr<SVerticalBox> GameOptionsBox;

	void RefreshGameOptionsList();

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
	TSharedRef<SWidget> GenerateGameModesList(UClass* InItem);

	TSharedRef<SWidget> GenerateStringListWidget(TSharedPtr<FString> InItem);

	/* Updates map drop-down box. */
	void OnMapSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	/* Updates game mode drop-down box. */
	void OnGameModeSelected(UClass* NewSelection, ESelectInfo::Type SelectInfo);
	
};