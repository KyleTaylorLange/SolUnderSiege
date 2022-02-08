// Copyright Kyle Taylor Lange

#pragma once
#include "SlateBasics.h"
#include "AssetData.h"
#include "SLastimWindow.h"

struct FServerEntry
{
	FString ServerName;
	FString CurrentPlayers;
	FString MaxPlayers;
	FString GameType;
	FString MapName;
	FString Ping;
	int32 SearchResultsIndex;
};

/**
 * Menu for setting options and creating new games.
 */
class SServerBrowserWidget : public SLastimWindow//, public FGCObject
{
public:

	virtual void WindowSetup() override;

	virtual TSharedRef<SWidget> ConstructWindow() override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual void UpdateSearchStatus();

	FText GetStatusText() const;

	/** creates single item widget, called for every list item */
	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FServerEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/* Set current server upon click. */
	void EntrySelectionChanged(TSharedPtr<FServerEntry> InItem, ESelectInfo::Type SelectInfo);

	/* Connect to server if double clicked. */
	void OnListItemDoubleClicked(TSharedPtr<FServerEntry> InItem);

protected:

	FText StatusText;

	/* Test: search for LAN servers. */
	virtual FReply QueryLAN();

	/* Test: search for internet servers. */
	virtual FReply QueryInternet();

	/** List of servers */
	TArray< TSharedPtr<FServerEntry> > ServerList;

	/** Widget displaying servers. */
	TSharedPtr< SListView< TSharedPtr<FServerEntry> > > ServerListWidget;

	bool bSearchingForServers;

	void OnServerSearchFinished();

	void UpdateServerList();

	/** currently selected list item */
	TSharedPtr<FServerEntry> SelectedServer;

	/* Connect to the currently selected server. */
	void ConnectToServer();
};