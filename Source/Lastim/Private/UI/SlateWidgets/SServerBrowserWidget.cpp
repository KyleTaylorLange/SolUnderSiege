// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SComboBox.h"
#include "STextComboBox.h"
#include "MainMenuHUD.h"
#include "SolLocalPlayer.h"
#include "SolGameInstance.h"
#include "SolGameSession.h"
#include "SServerBrowserWidget.h"
#include "SlateOptMacros.h"

#include "UserWidget.h"

#define LOCTEXT_NAMESPACE "ServerBrowser"

void SServerBrowserWidget::WindowSetup()
{
	StatusText = LOCTEXT("Initializing", "INITIALIZING...");
	bSearchingForServers = false;
}

TSharedRef<SWidget> SServerBrowserWidget::ConstructWindow()
{
	TSharedPtr<SVerticalBox> TestBox = SNew(SVerticalBox);

	TestBox->AddSlot()//.AutoHeight().VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.Padding(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.5f, 0.5f, 0.5f, 0.5f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SNew(SBox)
				.WidthOverride(800)
				.HeightOverride(600)
				.HAlign(HAlign_Center)
				[
					SAssignNew(ServerListWidget, SListView<TSharedPtr<FServerEntry>>)
					.ItemHeight(20)
					.ListItemsSource(&ServerList)
					.SelectionMode(ESelectionMode::Single)
					.OnGenerateRow(this, &SServerBrowserWidget::MakeListViewWidget)
					.OnSelectionChanged(this, &SServerBrowserWidget::EntrySelectionChanged)
					.OnMouseButtonDoubleClick(this, &SServerBrowserWidget::OnListItemDoubleClicked)
					.HeaderRow(
						SNew(SHeaderRow)
						+ SHeaderRow::Column("ServerName").FixedWidth(250).DefaultLabel(NSLOCTEXT("ServerList", "ServerNameColumn", "Server Name"))
						+ SHeaderRow::Column("GameType").DefaultLabel(NSLOCTEXT("ServerList", "GameTypeColumn", "Game Type"))
						+ SHeaderRow::Column("Map").DefaultLabel(NSLOCTEXT("ServerList", "MapNameColumn", "Map"))
						+ SHeaderRow::Column("Players").DefaultLabel(NSLOCTEXT("ServerList", "PlayersColumn", "Players"))
						+ SHeaderRow::Column("Ping").DefaultLabel(NSLOCTEXT("ServerList", "NetworkPingColumn", "Ping")))
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SBox)
				.WidthOverride(600)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::White)
					.ShadowColorAndOpacity(FLinearColor::Black)
					.ShadowOffset(FIntPoint(-1, 1))
					.Text(this, &SServerBrowserWidget::GetStatusText)
				]
			]
		]
	];
	TestBox->AddSlot()//.AutoHeight().VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.Padding(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.5f, 0.5f, 0.5f, 0.5f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SServerBrowserWidget::QueryLAN)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::White)
					.ShadowColorAndOpacity(FLinearColor::Black)
					.ShadowOffset(FIntPoint(-1, 1))
					.Text(FText::FromString(FString("Query LAN")))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &SServerBrowserWidget::QueryInternet)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FLinearColor::White)
					.ShadowColorAndOpacity(FLinearColor::Black)
					.ShadowOffset(FIntPoint(-1, 1))
					.Text(FText::FromString(FString("Query Internet")))
					.ToolTip(SNew(SToolTip)
							[
								SNew(STextBlock)
								.Text(FText::FromString(FString("Test tooltip, please ignore.")))
							])
				]
			]
		]
	];

	return TestBox.ToSharedRef();
}

void SServerBrowserWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bSearchingForServers)
	{
		UpdateSearchStatus();
	}
}

void SServerBrowserWidget::UpdateSearchStatus()
{
	check(bSearchingForServers); // should not be called otherwise

	bool bFinishSearch = true;

	// Too lazy to write this as a function yet.
	//ASolGameSession* ShooterSession = GetGameSession();
	ASolGameSession* ShooterSession = nullptr;
	USolGameInstance* const GI = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		ShooterSession = GI->GetGameSession();
	}

	if (ShooterSession)
	{
		int32 CurrentSearchIdx, NumSearchResults;
		EOnlineAsyncTaskState::Type SearchState = ShooterSession->GetSearchResultStatus(CurrentSearchIdx, NumSearchResults);

		UE_LOG(LogOnlineGame, Log, TEXT("Lastim->GetSearchResultStatus: %s"), EOnlineAsyncTaskState::ToString(SearchState));
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("GetSearchResultStatus: %s, Results: %d"), EOnlineAsyncTaskState::ToString(SearchState), NumSearchResults));

		switch (SearchState)
		{
		case EOnlineAsyncTaskState::InProgress:
		{
			StatusText = LOCTEXT("Searching", "SEARCHING...");
			StatusText = FText::FromString(FString::Printf(TEXT("SEARCHING: %d servers found."), NumSearchResults));

			const TArray<FOnlineSessionSearchResult> & SearchResults = ShooterSession->GetSearchResults();
			check(SearchResults.Num() == NumSearchResults);
			if (ServerList.Num() != NumSearchResults)
			{
				for (int32 IdxResult = 0; IdxResult < NumSearchResults; ++IdxResult)
				{
					if (!ServerList.IsValidIndex(IdxResult))
					{
						TSharedPtr<FServerEntry> NewServerEntry = MakeShareable(new FServerEntry());

						const FOnlineSessionSearchResult& Result = SearchResults[IdxResult];

						NewServerEntry->ServerName = Result.Session.OwningUserName;
						NewServerEntry->Ping = FString::FromInt(Result.PingInMs);
						NewServerEntry->CurrentPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections
							+ Result.Session.SessionSettings.NumPrivateConnections
							- Result.Session.NumOpenPublicConnections
							- Result.Session.NumOpenPrivateConnections);
						NewServerEntry->MaxPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections
							+ Result.Session.SessionSettings.NumPrivateConnections);
						NewServerEntry->SearchResultsIndex = IdxResult;

						Result.Session.SessionSettings.Get(SETTING_GAMEMODE, NewServerEntry->GameType);
						Result.Session.SessionSettings.Get(SETTING_MAPNAME, NewServerEntry->MapName);

						ServerList.Add(NewServerEntry);
					}
				}
				UpdateServerList();
			}

			bFinishSearch = false;
		}
		break;

		case EOnlineAsyncTaskState::Done:
		{
			// Copy the results.
			StatusText = FText::FromString(FString::Printf(TEXT("COMPLETED: %d servers found."), NumSearchResults));
			if (NumSearchResults == 0)
			{
				StatusText = LOCTEXT("NoServersFound", "NO SERVERS FOUND");
				//StatusText = LOCTEXT("NoServersFound", "NO SERVERS FOUND, PRESS SPACE TO TRY AGAIN");
			}
			else
			{
				StatusText = LOCTEXT("ServersRefresh", "PRESS SPACE TO REFRESH SERVER LIST");
			}
		}
		break;

		case EOnlineAsyncTaskState::Failed:
			// intended fall-through
		case EOnlineAsyncTaskState::NotStarted:
			StatusText = FText::GetEmpty();
			// intended fall-through
		default:
			break;
		}
	}

	if (bFinishSearch)
	{
		OnServerSearchFinished();
	}
}

/** Called when server search is finished */
void SServerBrowserWidget::OnServerSearchFinished()
{
	bSearchingForServers = false;
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("Finished Server Search!")));
	UpdateServerList();

	// Too lazy to write this as a function yet.
	//ASolGameSession* ShooterSession = GetGameSession();
	ASolGameSession* ShooterSession = nullptr;
	USolGameInstance* const GI = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		ShooterSession = GI->GetGameSession();
	}
	if (ShooterSession)
	{
		int32 CurrentSearchIdx, NumSearchResults;
		EOnlineAsyncTaskState::Type SearchState = ShooterSession->GetSearchResultStatus(CurrentSearchIdx, NumSearchResults);

		const TArray<FOnlineSessionSearchResult> & SearchResults = ShooterSession->GetSearchResults();
		check(SearchResults.Num() == NumSearchResults);
		if (NumSearchResults == 0)
		{
			return;
		}
		else
		{
			/*
			FOnlineSessionSearchResult ServerResult = ShooterSession->GetSearchResults()[0];
			
			
			
			bool bJoined = GI->JoinSession(PlayerOwner.Get(), ServerResult); //GI->JoinSession(PlayerOwner.Get(), 0);
			{
				if (bJoined)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("JoinSession: Succeeded!")));
				}
				else
				{
					//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("JoinSession: Failed!")));
				}
			}
			*/
		}
	}
}

void SServerBrowserWidget::UpdateServerList()
{
	/*
	// Only filter maps if a specific map is specified.
	if (MapFilterName != "Any")
	{
		for (int32 i = 0; i < ServerList.Num(); ++i)
		{
			// Only filter maps if a specific map is specified.
			if (ServerList[i]->MapName != MapFilterName)
			{
				ServerList.RemoveAt(i);
			}
		}
	}
	*/
	int32 SelectedItemIndex = -1; //ServerList.IndexOfByKey(SelectedItem);

	ServerListWidget->RequestListRefresh();
	if (ServerList.Num() > 0)
	{
		ServerListWidget->UpdateSelectionSet();
		ServerListWidget->SetSelection(ServerList[SelectedItemIndex > -1 ? SelectedItemIndex : 0], ESelectInfo::OnNavigation);
	}
}

void SServerBrowserWidget::EntrySelectionChanged(TSharedPtr<FServerEntry> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedServer = InItem;
}

void SServerBrowserWidget::OnListItemDoubleClicked(TSharedPtr<FServerEntry> InItem)
{
	SelectedServer = InItem;
	ConnectToServer();
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
}

void SServerBrowserWidget::ConnectToServer()
{
	if (bSearchingForServers)
	{
		// unsafe
		return;
	}
#if WITH_EDITOR
	if (GIsEditor == true)
	{
		return;
	}
#endif
	if (SelectedServer.IsValid())
	{
		int ServerToJoin = SelectedServer->SearchResultsIndex;

		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveAllViewportWidgets();
		}

		USolGameInstance* const GI = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
		if (GI)
		{
			GI->JoinSession(PlayerOwner.Get(), ServerToJoin);
		}
	}
}

FReply SServerBrowserWidget::QueryLAN()
{
	// TEST: Immediately start search when window opens.
	USolGameInstance* const GI = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		ServerList.Empty();
		GI->FindSessions(PlayerOwner.Get(), true);
		bSearchingForServers = true;
	}
	
	return FReply::Handled();
}

FReply SServerBrowserWidget::QueryInternet()
{
	// TEST: Immediately start search when window opens.
	USolGameInstance* const GI = Cast<USolGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		ServerList.Empty();
		GI->FindSessions(PlayerOwner.Get(), false);
		bSearchingForServers = true;
	}

	return FReply::Handled();
}

FText SServerBrowserWidget::GetStatusText() const
{
	return StatusText;
}


TSharedRef<ITableRow> SServerBrowserWidget::MakeListViewWidget(TSharedPtr<FServerEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	class SServerEntryWidget : public SMultiColumnTableRow< TSharedPtr<FServerEntry> >
	{
	public:
		SLATE_BEGIN_ARGS(SServerEntryWidget) {}
		SLATE_END_ARGS()

			void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FServerEntry> InItem)
		{
			Item = InItem;
			SMultiColumnTableRow< TSharedPtr<FServerEntry> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			FText ItemText = FText::GetEmpty();
			if (ColumnName == "ServerName")
			{
				ItemText = FText::FromString(Item->ServerName);
			}
			else if (ColumnName == "GameType")
			{
				ItemText = FText::FromString(Item->GameType);
			}
			else if (ColumnName == "Map")
			{
				ItemText = FText::FromString(Item->MapName);
			}
			else if (ColumnName == "Players")
			{
				ItemText = FText::Format(FText::FromString("{0}/{1}"), FText::FromString(Item->CurrentPlayers), FText::FromString(Item->MaxPlayers));
			}
			else if (ColumnName == "Ping")
			{
				ItemText = FText::FromString(Item->Ping);
			}
			return SNew(STextBlock)
				.Text(ItemText);
				//.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuServerListTextStyle");
		}
		TSharedPtr<FServerEntry> Item;
	};
	return SNew(SServerEntryWidget, OwnerTable, Item);
}

#undef LOCTEXT_NAMESPACE