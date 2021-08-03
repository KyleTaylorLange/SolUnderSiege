// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/PlayerController.h"
#include "SolCharacter.h"
#include "SolSpectatorPawn.h"
#include "SolHUD.h"
#include "SolPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ASolPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	virtual void SetupInputComponent() override;

	virtual void ReceivedPlayer() override;

	/** Returns a pointer to the current Lastim Character. May return NULL. **/
	ASolCharacter* GetSolCharacter() const;

	/** Returns a pointer to the current Lastim Spectator Pawn. May return NULL. **/
	ASolSpectatorPawn* GetSolSpectatorPawn() const;

	/* Finds a static spot for the camera upon death.
		Copied from ShooterGame and not ideal, but works for now. */
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	// When player presses (and releases) fire.
	void OnFire();
	void OnStopFire();

	// When player presses (and releases) aim.
	void OnAim();
	void OnStopAim();

	// When player presses (and releases) use.
	void OnUse();
	void OnStopUse();

	// When player presses (and releases) reload.
	void OnReload();
	void OnStopReload();

	// When player presses (and releases) reload.
	void OnSwitchFireMode();
	void OnStopSwitchFireMode();

	// When move forward (value = 1) or backward (value = -1) is held down.
	void MoveForward(float Value);

	// When move forward (value = 1) or backward (value = -1) is held down.
	void MoveRight(float Value);

	// When moving up (value = 1) or down (value = -1) is held down.
	void MoveUp(float Value);

	// When player presses (and releases) jump.
	void OnJump();
	void OnStopJump();

	// When player presses (and releases) crouch.
	void OnCrouch();
	void OnStopCrouch();

	// When player presses (and releases) crouch.
	void OnSprint();
	void OnStopSprint();

	/** shows scoreboard */
	void OnOpenInventory();

	/** shows scoreboard */
	void OnToggleInGameMenu();

	/** shows scoreboard */
	void OnToggleScoreboard();

	/** shows scoreboard */
	void OnShowScoreboard();

	/** hides scoreboard */
	void OnHideScoreboard();

	/* Current weapon selection index. */
	int32 WeapSelectIndex;

	/* Is the player in the weapon selection menu? */
	bool bSelectingWeapon;

	void OnPreviousWeapon();

	void OnNextWeapon();

	void BeginSelectWeapon();

	void AbortSelectWeapon();

	void FinishSelectWeapon(int32 DeltaIndex);

	void IncrementWeapSelectIndex(int32 Amount);

	UFUNCTION(exec)
	void SetColors(FLinearColor Primary, FLinearColor Secondary);

public:

	UFUNCTION(Exec)
	void Say(const FString& Msg);

	UFUNCTION(Exec)
	void TeamSay(const FString& Msg);

	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerSay(const FString& Msg, FName Type);

	// Sends a message to all players.
	UFUNCTION(Reliable, Client)
	virtual void ClientSendMessage(class ASolPlayerState* SenderPlayerState, const FString& S, FName Type);

	/* Basically called whenever the player dies. */
	void PawnPendingDestroy(APawn* P) override;

	/* Begin waiting if we cannot spawn. */
	virtual void BeginWaiting();

	virtual bool CanRestartPlayer() override;

	virtual void UpdateRotation(float DeltaTime) override;

	/* Sets a static location for the player's death camera. 
		Copied from ShooterGame and not ideal, but works for now. */
	UFUNCTION(Reliable, Client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

	/* Writes a death message to the player's HUD. */
	UFUNCTION(Reliable, Client)
	void OnDeathMessage(class ASolPlayerState* KillerPlayerState, class ASolPlayerState* KilledPlayerState, const UDamageType* KillerDamageType);

	/* Sets the player's colors. */
	UFUNCTION(Reliable, Server)
	void SetPlayerColors(FLinearColor Primary, FLinearColor Secondary);
};
