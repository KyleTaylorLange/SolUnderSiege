// Copyright Kyle Taylor Lange
// Most code taken directly from ShooterGame example project.

#include "Lastim.h"
#include "Online.h"
#include "OnlineAchievementsInterface.h"
#include "OnlineEventsInterface.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"
#include "SolGameSession.h"

ASolGameSession::ASolGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ASolGameSession::OnCreateSessionComplete);
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ASolGameSession::OnDestroySessionComplete);

		OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ASolGameSession::OnFindSessionsComplete);
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &ASolGameSession::OnJoinSessionComplete);

		//OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &AShooterGameSession::OnStartOnlineGameComplete);
	}
}

bool ASolGameSession::HostSession(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName, const FString& GameType, const FString& MapName, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers)
{
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		/*
		CurrentSessionParams.SessionName = InSessionName;
		CurrentSessionParams.bIsLAN = bIsLAN;
		CurrentSessionParams.bIsPresence = bIsPresence;
		CurrentSessionParams.UserId = UserId;
		MaxPlayers = MaxNumPlayers;
		*/
		CurrentSessionParamsUserId = UserId;

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		//if (Sessions.IsValid() && CurrentSessionParams.UserId.IsValid())
		if (Sessions.IsValid() && UserId.IsValid())
		{
			TSharedPtr<class FOnlineSessionSettings> HostSettings = MakeShareable(new FOnlineSessionSettings());
			//HostSettings = MakeShareable(new FShooterOnlineSessionSettings(bIsLAN, bIsPresence, MaxPlayers));

			HostSettings->NumPublicConnections = FMath::Max(0, MaxNumPlayers);
			HostSettings->NumPrivateConnections = 0;
			HostSettings->bIsLANMatch = bIsLAN;
			HostSettings->bShouldAdvertise = true;
			HostSettings->bAllowJoinInProgress = true;
			HostSettings->bAllowInvites = true;
			HostSettings->bUsesPresence = bIsPresence;
			HostSettings->bAllowJoinViaPresence = true;
			HostSettings->bAllowJoinViaPresenceFriendsOnly = false;

			HostSettings->Set(SETTING_GAMEMODE, GameType, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MAPNAME, MapName, EOnlineDataAdvertisementType::ViaOnlineService);
			//TODO: Fix these for 4.15.
			//HostSettings->Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
			//HostSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			//HostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
			//HostSettings->Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);

			OnCreateSessionCompleteDelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			//return Sessions->CreateSession(*CurrentSessionParams.UserId, CurrentSessionParams.SessionName, *HostSettings);
			return Sessions->CreateSession(*CurrentSessionParamsUserId, InSessionName, *HostSettings);
		}
	}
#if !UE_BUILD_SHIPPING
	else
	{
		// Hack workflow in development
		//OnCreatePresenceSessionComplete().Broadcast(GameSessionName, true);
		return true;
	}
#endif
	return false;
}

void ASolGameSession::FindSessions(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName, bool bIsLAN, bool bIsPresence)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		/*
		CurrentSessionParams.SessionName = SessionName;
		CurrentSessionParams.bIsLAN = bIsLAN;
		CurrentSessionParams.bIsPresence = bIsPresence;
		CurrentSessionParams.UserId = UserId;
		*/
		CurrentSessionParamsUserId = UserId;

		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && CurrentSessionParamsUserId.IsValid())
		{
			SearchSettings = MakeShareable(new FOnlineSessionSearch);
			SearchSettings->bIsLanQuery = bIsLAN;
			SearchSettings->MaxSearchResults = 10;
			SearchSettings->PingBucketSize = 50;
			if (bIsPresence)
			{
				SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
			}
			//SearchSettings = MakeShareable(new FShooterOnlineSearchSettings(bIsLAN, bIsPresence));
			//SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, CustomMatchKeyword, EOnlineComparisonOp::Equals);

			TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SearchSettings.ToSharedRef();

			OnFindSessionsCompleteDelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
			Sessions->FindSessions(*CurrentSessionParamsUserId, SearchSettingsRef);
		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}
}

void ASolGameSession::OnFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			UE_LOG(LogOnlineGame, Verbose, TEXT("Num Search Results: %d"), SearchSettings->SearchResults.Num());
			for (int32 SearchIdx = 0; SearchIdx < SearchSettings->SearchResults.Num(); SearchIdx++)
			{
				const FOnlineSessionSearchResult& SearchResult = SearchSettings->SearchResults[SearchIdx];
				DumpSession(&SearchResult.Session);
			}

			OnFindSessionsComplete().Broadcast(bWasSuccessful);
		}
	}
}

bool ASolGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName, int32 SessionIndexInSearchResults)
{
	bool bResult = false;

	if (SessionIndexInSearchResults >= 0 && SessionIndexInSearchResults < SearchSettings->SearchResults.Num())
	{
		bResult = JoinSession(UserId, InSessionName, SearchSettings->SearchResults[SessionIndexInSearchResults]);
	}

	return bResult;
}

bool ASolGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserId, FName InSessionName, const FOnlineSessionSearchResult& SearchResult)
{
	bool bResult = false;

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid() && UserId.IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
			UE_LOG(LogOnlineGame, Log, TEXT("SolGameSession::JoinSession: Joining Session"));
			bResult = Sessions->JoinSession(*UserId, InSessionName, SearchResult);
		}
	}

	return bResult;
}

void ASolGameSession::OnJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = NULL;
	if (OnlineSub)
	{
		Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	OnJoinSessionComplete().Broadcast(Result);
}

void ASolGameSession::OnCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
	}

	OnCreatePresenceSessionComplete().Broadcast(InSessionName, bWasSuccessful);
}

void ASolGameSession::OnDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnlineGame, Verbose, TEXT("ASolGameSession::OnDestroySessionComplete %s bSuccess: %d"), *InSessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		//HostSettings = NULL;
	}
}

const TArray<FOnlineSessionSearchResult> & ASolGameSession::GetSearchResults() const
{
	return SearchSettings->SearchResults;
};

EOnlineAsyncTaskState::Type ASolGameSession::GetSearchResultStatus(int32& SearchResultIdx, int32& NumSearchResults)
{
	SearchResultIdx = 0;
	NumSearchResults = 0;

	if (SearchSettings.IsValid())
	{
		if (SearchSettings->SearchState == EOnlineAsyncTaskState::Done)
		{
			//SearchResultIdx = CurrentSessionParams.BestSessionIdx;
			NumSearchResults = SearchSettings->SearchResults.Num();
		}
		else if (SearchSettings->SearchState == EOnlineAsyncTaskState::InProgress)
		{
			NumSearchResults = SearchSettings->SearchResults.Num();
		}
		return SearchSettings->SearchState;
	}

	return EOnlineAsyncTaskState::NotStarted;
}

