// Copyright Kyle Taylor Lange
// Most code taken from ShooterGame example project.

#include "Lastim.h"
#include "OnlineSubsystemUtils.h"
#include "SolGameInstance.h"

USolGameInstance::USolGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	//, bIsOnline(true) // Default to online
	//, bIsLicensed(true) // Default to licensed (should have been checked by OS on boot)
{
	CurrentState = SolGameInstanceState::None;
}

bool USolGameInstance::Tick(float DeltaSeconds)
{
	MaybeChangeState();
	
	return true;
}

void USolGameInstance::Init()
{
	Super::Init();

	// game requires the ability to ID users.
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	check(OnlineSub);
	const IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
	check(IdentityInterface.IsValid());
	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());

	// Register delegate for ticker callback
	TickDelegate = FTickerDelegate::CreateUObject(this, &USolGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);

	if (SessionInterface.IsValid())
	{
		SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &USolGameInstance::HandleSessionFailure));
	}
}

void USolGameInstance::StartGameInstance()
{
	Super::StartGameInstance();
	/* Go to the main menu state. */
	GoToState(GetInitialState());
}

FName USolGameInstance::GetInitialState()
{
	/* On PC, we always want to go to the main menu.
	   If we ever make a console version, we might want to go to a welcome screen first. */
	return SolGameInstanceState::MainMenu;
}

void USolGameInstance::GoToState(FName NewState)
{
	PendingState = NewState;
}

void USolGameInstance::MaybeChangeState()
{
	if ((PendingState != CurrentState) && (PendingState != SolGameInstanceState::None))
	{
		// end current state
		EndCurrentState();

		// begin new state
		BeginNewState(PendingState);

		// clear pending change
		PendingState = SolGameInstanceState::None;
	}
}

void USolGameInstance::EndCurrentState()
{
	if (CurrentState == SolGameInstanceState::MainMenu)
	{
		EndMainMenuState();
	}
	else if (CurrentState == SolGameInstanceState::Playing)
	{
		EndPlayingState();
	}
	
	CurrentState = SolGameInstanceState::None;
}

void USolGameInstance::BeginNewState(FName NewState)
{
	if (NewState == SolGameInstanceState::MainMenu)
	{
		BeginMainMenuState();
	}
	else if (NewState == SolGameInstanceState::Playing)
	{
		BeginPlayingState();
	}

	CurrentState = NewState;
}

void USolGameInstance::BeginMainMenuState()
{
	/* Open main menu map if not already open. */
	LoadFrontEndMap(MainMenuMap);
}

void USolGameInstance::EndMainMenuState()
{
	/* Delete any open sessions so we can create a new one. */
	CleanupSessionOnReturnToMenu();
}

void USolGameInstance::BeginPlayingState()
{
	// Make sure viewport has focus
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void USolGameInstance::EndPlayingState()
{
	CleanupSessionOnReturnToMenu();
}

ASolGameSession* USolGameInstance::GetGameSession() const
{
	UWorld* const World = GetWorld();
	if (World)
	{
		AGameModeBase* const Game = World->GetAuthGameMode();
		if (Game)
		{
			return Cast<ASolGameSession>(Game->GameSession);
		}
	}

	return nullptr;
}

/** Initiates the session searching */
bool USolGameInstance::FindSessions(ULocalPlayer* PlayerOwner, bool bFindLAN)
{
	bool bResult = false;

	check(PlayerOwner != nullptr);
	if (PlayerOwner)
	{
		ASolGameSession* const GameSession = GetGameSession();
		if (GameSession)
		{
			GameSession->OnFindSessionsComplete().RemoveAll(this);
			OnSearchSessionsCompleteDelegateHandle = GameSession->OnFindSessionsComplete().AddUObject(this, &USolGameInstance::OnSearchSessionsComplete);

			GameSession->FindSessions(PlayerOwner->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, bFindLAN, true);

			bResult = true;
		}
	}

	return bResult;
}

bool USolGameInstance::HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL)
{
	/*
	if (!GetIsOnline())
	{
		//
		// Offline game, just go straight to map
		//

		//ShowLoadingScreen();
		//GotoState(ShooterGameInstanceState::Playing);

		// Travel to the specified match URL
		TravelURL = InTravelURL;
		GetWorld()->ServerTravel(TravelURL);
		return true;
	}
	*/

	//
	// Online game
	//

	ASolGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		// add callback delegate for completion
		OnCreatePresenceSessionCompleteDelegateHandle = GameSession->OnCreatePresenceSessionComplete().AddUObject(this, &USolGameInstance::OnCreatePresenceSessionComplete);

		TravelURL = InTravelURL;
		bool const bIsLanMatch = InTravelURL.Contains(TEXT("?bIsLanMatch"));

		//determine the map name from the travelURL
		const FString& MapNameSubStr = "/Game/Maps/";
		const FString& ChoppedMapName = TravelURL.RightChop(MapNameSubStr.Len());
		const FString& MapName = ChoppedMapName.LeftChop(ChoppedMapName.Len() - ChoppedMapName.Find("?game"));

		if (GameSession->HostSession(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, GameType, MapName, bIsLanMatch, true, 32/*AShooterGameSession::DEFAULT_NUM_PLAYERS*/))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == SolGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				//ShowLoadingScreen();
				GoToState(SolGameInstanceState::Playing);
				return true;
			}
		}
	}
	return false;
}

/** Callback which is intended to be called upon session creation */
void USolGameInstance::OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful)
{
	////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("GameInstance->OnCreatePresenceSessionComplete")));
	ASolGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		FinishSessionCreation(bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
		/*
		GameSession->OnCreatePresenceSessionComplete().Remove(OnCreatePresenceSessionCompleteDelegateHandle);

		// Add the splitscreen player if one exists
		if (bWasSuccessful && LocalPlayers.Num() > 1)
		{
			auto Sessions = Online::GetSessionInterface();
			if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
			{
				Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName,
					FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UShooterGameInstance::OnRegisterLocalPlayerComplete));
			}
		}
		else
		{
			// We either failed or there is only a single local user
			FinishSessionCreation(bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
		}
		*/
	}
}

void USolGameInstance::FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// Travel to the specified match URL
		UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::FinishSessionCreation: travelling to server."));
		GetWorld()->ServerTravel(TravelURL);
	}
	else
	{
		UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::FinishSessionCreation: failed to create session."));
		/*
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
		*/
	}
}

bool USolGameInstance::JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults)
{
	// needs to tear anything down based on current state?
	UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::JoinSessionA: Begin"));
	ASolGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		//AddNetworkFailureHandlers();

		OnJoinSessionCompleteDelegateHandle = GameSession->OnJoinSessionComplete().AddUObject(this, &USolGameInstance::OnJoinSessionComplete);
		if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, SessionIndexInSearchResults))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == SolGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				//ShowLoadingScreen();
				GoToState(SolGameInstanceState::Playing);
				UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::JoinSessionA: True"));
				return true;
			}
		}
	}
	UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::JoinSessionA: False"));
	return false;
}

bool USolGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	// needs to tear anything down based on current state?
	UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::JoinSessionB: Begin"));
	ASolGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		//AddNetworkFailureHandlers();

		OnJoinSessionCompleteDelegateHandle = GameSession->OnJoinSessionComplete().AddUObject(this, &USolGameInstance::OnJoinSessionComplete);
		if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, SearchResult))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == SolGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				//ShowLoadingScreen();
				GoToState(SolGameInstanceState::Playing);
				UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::JoinSessionA: True"));
				return true;
			}
		}
	}
	UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::JoinSessionB: False"));
	return false;
}

/** Callback which is intended to be called upon finding sessions */
void USolGameInstance::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogOnlineGame, Log, TEXT("SolGameInstance::OnJoinSessionComplete: Begin"));
	// unhook the delegate
	ASolGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		GameSession->OnJoinSessionComplete().Remove(OnJoinSessionCompleteDelegateHandle);
	}

	// Add the splitscreen player if one exists
	if (Result == EOnJoinSessionCompleteResult::Success && LocalPlayers.Num() > 1)
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
		{
			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName,
				FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &USolGameInstance::OnRegisterJoiningLocalPlayerComplete));
		}
	}
	else
	{
		// We either failed or there is only a single local user
		FinishJoinSession(Result);
	}
	UE_LOG(LogOnlineGame, Log, TEXT("GI->OnJoinSessionComplete: End"));
}

void USolGameInstance::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogOnlineGame, Log, TEXT("GI->FinishJoinSession: Begin"));
	////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("GI->FinishJoinSession: Begin")));
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		FText ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game is full.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game no longer exists.");
			break;
		default:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.");
			break;
		}

		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		//RemoveNetworkFailureHandlers();
		//ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
		return;
	}
	UE_LOG(LogOnlineGame, Log, TEXT("GI->FinishJoinSession: End"));
	////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("GI->FinishJoinSession: End")));
	InternalTravelToSession(GameSessionName);
}

void USolGameInstance::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogOnlineGame, Warning, TEXT("USolGameInstance::HandleSessionFailure: %u"), (uint32)FailureType);
}

void USolGameInstance::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishJoinSession(Result);
}

void USolGameInstance::InternalTravelToSession(const FName& SessionName)
{
	////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("GI->InternalTravelToSession: Begin")));
	APlayerController * const PlayerController = GetFirstLocalPlayerController();
	UE_LOG(LogOnlineGame, Log, TEXT("GI->InternalTravelToSession: Begin"));
	if (PlayerController == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "InvalidPlayerController", "Invalid Player Controller");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		//RemoveNetworkFailureHandlers();
		//ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	// travel to session
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "OSSMissing", "OSS missing");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		//RemoveNetworkFailureHandlers();
		//ShowMessageThenGoMain(ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	FString URL;
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	if (!Sessions.IsValid() || !Sessions->GetResolvedConnectString(SessionName, URL))
	{
		FText FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		//ShowMessageThenGoMain(FailReason, OKButton, FText::GetEmpty());
		UE_LOG(LogOnlineGame, Warning, TEXT("Failed to travel to session upon joining it"));
		return;
	}
	UE_LOG(LogOnlineGame, Log, TEXT("GI->InternalTravelToSession: End"));
	////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("GI->InternalTravelToSession: End")));
	PlayerController->ClientTravel(URL, TRAVEL_Absolute);
}

bool USolGameInstance::LoadFrontEndMap(const FString& MapName)
{
	bool bSuccess = true;

	// if already loaded, do nothing
	UWorld* const World = GetWorld();
	if (World)
	{
		FString const CurrentMapName = *World->PersistentLevel->GetOutermost()->GetName();
		//if (MapName.Find(TEXT("Highrise")) != -1)
		if (CurrentMapName == MapName)
		{
			return bSuccess;
		}
	}

	FString Error;
	EBrowseReturnVal::Type BrowseRet = EBrowseReturnVal::Failure;
	FURL URL(*FString::Printf(TEXT("%s"), *MapName));

	if (URL.Valid && !HasAnyFlags(RF_ClassDefaultObject)) //CastChecked<UEngine>() will fail if using Default__ShooterGameInstance, so make sure that we're not default
	{
		BrowseRet = GetEngine()->Browse(*WorldContext, URL, Error);

		// Handle failure.
		if (BrowseRet != EBrowseReturnVal::Success)
		{
			UE_LOG(LogLoad, Fatal, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), *MapName, *Error));
			bSuccess = false;
		}
	}
	return bSuccess;
}

void USolGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Log, TEXT("USolGameInstance::OnEndSessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
			Sessions->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegateHandle);
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		}
	}

	// continue
	CleanupSessionOnReturnToMenu();
}

void USolGameInstance::CleanupSessionOnReturnToMenu()
{
	// end online game and then destroy it
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

	if (Sessions.IsValid())
	{
		FName GameSession(NAME_GameSession);
		EOnlineSessionState::Type SessionState = Sessions->GetSessionState(NAME_GameSession);
		UE_LOG(LogOnline, Log, TEXT("Session %s is '%s'"), *GameSession.ToString(), EOnlineSessionState::ToString(SessionState));

		if (EOnlineSessionState::InProgress == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Ending session %s on return to main menu"), *GameSession.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->EndSession(NAME_GameSession);
		}
		else if (EOnlineSessionState::Ending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to end on return to main menu"), *GameSession.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
		}
		else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Destroying session %s on return to main menu"), *GameSession.ToString());
			OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->DestroySession(NAME_GameSession);
		}
		else if (EOnlineSessionState::Starting == SessionState)
		{
			UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to start, and then we will end it to return to main menu"), *GameSession.ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
		}
	}
}

/** Callback which is intended to be called upon finding sessions */
void USolGameInstance::OnSearchSessionsComplete(bool bWasSuccessful)
{
	ASolGameSession* const Session = GetGameSession();
	if (Session)
	{
		Session->OnFindSessionsComplete().Remove(OnSearchSessionsCompleteDelegateHandle);
	}
}

namespace SolGameInstanceState
{
	const FName None = FName(TEXT("None"));
	const FName PendingInvite = FName(TEXT("PendingInvite"));
	const FName WelcomeScreen = FName(TEXT("WelcomeScreen"));
	const FName MainMenu = FName(TEXT("MainMenu"));
	const FName MessageMenu = FName(TEXT("MessageMenu"));
	const FName Playing = FName(TEXT("Playing"));
}
