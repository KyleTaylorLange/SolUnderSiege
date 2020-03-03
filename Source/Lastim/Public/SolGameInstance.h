// Copyright Kyle Taylor Lange

#pragma once

#include "Engine/GameInstance.h"
#include "SolGameSession.h"
#include "SolGameInstance.generated.h"

namespace SolGameInstanceState
{
	extern const FName None;
	extern const FName PendingInvite;
	extern const FName WelcomeScreen;
	extern const FName MainMenu;
	extern const FName MessageMenu;
	extern const FName Playing;
}

/**
 * 
 */
UCLASS(config = Game)
class LASTIM_API USolGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()
	
public:

	/* Ticks menu, mostly to update the player's state. */
	bool Tick(float DeltaSeconds);

	virtual void Init() override;
	virtual void StartGameInstance() override;

	/* Returns the ASolGameSession. */
	ASolGameSession* GetGameSession() const;

	bool HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL);
	bool JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults);
	bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult);

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** Initiates the session searching */
	bool FindSessions(ULocalPlayer* PlayerOwner, bool bLANMatch);

	/** Goes to a new game instance state.
		@param NewState: desired state.*/
	void GoToState(FName NewState);

private:

	UPROPERTY(config)
	FString MainMenuMap;

	/* Current GI state. */
	FName CurrentState;
	/* Desired GI state. Will switch to if different every tick. */
	FName PendingState;

	void MaybeChangeState();
	/* Returns initial state to boot to.*/
	FName GetInitialState();

	void EndCurrentState();
	void BeginNewState(FName NewState);

	void BeginMainMenuState();
	void EndMainMenuState();
	void BeginPlayingState();
	void EndPlayingState();

	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;

	/** Handle to various registered delegates.
		Copied from ShooterGame; might not need all of these. */
	FDelegateHandle TickDelegateHandle;
	FDelegateHandle TravelLocalSessionFailureDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnSearchSessionsCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;

	/** Callback which is intended to be called upon session creation */
	void OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Called after all the local players are registered */
	void FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon joining session */
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is called after adding local users to a session we're joining */
	void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered in a session we're joining */
	void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

	/** Travel directly to the named session */
	void InternalTravelToSession(const FName& SessionName);

	/** Travels to the front end map. */
	bool LoadFrontEndMap(const FString& MapName);

	/** Delegate for ending a session */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;

	/** Called upon ending a session. */
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Closes any sessions upon return to the main menu. */
	void CleanupSessionOnReturnToMenu();

	/** Callback which is intended to be called upon finding sessions */
	void OnSearchSessionsComplete(bool bWasSuccessful);
};
