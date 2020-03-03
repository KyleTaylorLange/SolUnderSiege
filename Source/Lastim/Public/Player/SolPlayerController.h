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

	/** Returns a pointer to the current Lastim Character. May return NULL. **/
	ASolCharacter* GetSolCharacter() const;

	/** Returns a pointer to the current Lastim Spectator Pawn. May return NULL. **/
	ASolSpectatorPawn* GetSolSpectatorPawn() const;

	/** Returns a pointer to the Lastim HUD. May return NULL. **/
	ASolHUD* GetSolHUD() const;

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

public:

	/* Basically called whenever the player dies. */
	void PawnPendingDestroy(APawn* P) override;

	/* Begin waiting if we cannot spawn. */
	virtual void BeginWaiting();

	virtual bool CanRestartPlayer() override;

	virtual void UpdateRotation(float DeltaTime) override;

	/* Sets a static location for the player's death camera. 
		Copied from ShooterGame and not ideal, but works for now. */
	UFUNCTION(reliable, client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

	/* Writes a death message to the player's HUD. */
	void OnDeathMessage(class ASolPlayerState* KillerPlayerState, class ASolPlayerState* KilledPlayerState, const UDamageType* KillerDamageType);
};
