// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolCharacter.h"
#include "SolSpectatorPawn.h"
#include "SolPlayerCameraManager.h"
#include "SolPlayerState.h"
#include "SolHUD.h"
#include "SolPlayerController.h"

ASolPlayerController::ASolPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ASolPlayerCameraManager::StaticClass();

	bSelectingWeapon = false;
	WeapSelectIndex = 0;
}

void ASolPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Fire", IE_Pressed, this, &ASolPlayerController::OnFire);
	InputComponent->BindAction("Fire", IE_Released, this, &ASolPlayerController::OnStopFire);

	InputComponent->BindAction("Aim", IE_Pressed, this, &ASolPlayerController::OnAim);
	InputComponent->BindAction("Aim", IE_Released, this, &ASolPlayerController::OnStopAim);

	InputComponent->BindAction("Use", IE_Pressed, this, &ASolPlayerController::OnUse);
	InputComponent->BindAction("Use", IE_Released, this, &ASolPlayerController::OnStopUse);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ASolPlayerController::OnJump);
	InputComponent->BindAction("Jump", IE_Released, this, &ASolPlayerController::OnStopJump);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &ASolPlayerController::OnCrouch);
	InputComponent->BindAction("Crouch", IE_Released, this, &ASolPlayerController::OnStopCrouch);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &ASolPlayerController::OnSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &ASolPlayerController::OnStopSprint);

	InputComponent->BindAction("OpenInventory", IE_Pressed, this, &ASolPlayerController::OnOpenInventory);

	InputComponent->BindAction("ToggleInGameMenu", IE_Pressed, this, &ASolPlayerController::OnToggleInGameMenu);

	InputComponent->BindAction("ToggleScoreboard", IE_Pressed, this, &ASolPlayerController::OnToggleScoreboard);

	InputComponent->BindAction("ShowScoreboard", IE_Pressed, this, &ASolPlayerController::OnShowScoreboard);
	InputComponent->BindAction("ShowScoreboard", IE_Released, this, &ASolPlayerController::OnHideScoreboard);

	InputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &ASolPlayerController::OnPreviousWeapon);
	InputComponent->BindAction("NextWeapon", IE_Pressed, this, &ASolPlayerController::OnNextWeapon);
}

ASolCharacter* ASolPlayerController::GetSolCharacter() const
{
	return Cast<ASolCharacter>(GetCharacter());
}

ASolSpectatorPawn* ASolPlayerController::GetSolSpectatorPawn() const
{
	return Cast<ASolSpectatorPawn>(GetSpectatorPawn());
}

ASolHUD* ASolPlayerController::GetSolHUD() const
{
	return Cast<ASolHUD>(GetHUD());
}

void ASolPlayerController::OnDeathMessage(class ASolPlayerState* KillerPlayerState, class ASolPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->AddDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
	}
}

bool ASolPlayerController::CanRestartPlayer()
{
	ASolPlayerState* PS = Cast<ASolPlayerState>(PlayerState);
	if (PS)
	{
		return Super::CanRestartPlayer() && PS->RespawnTime <= 0.0f;
	}
	return false;
}

void ASolPlayerController::UpdateRotation(float DeltaTime)
{
	/* Below code is ripped directly from superclass.
		It is currently UNMODIFIED, but we will probably need to modify it mid-function (after DeltaRot).
		Hence, we might not be able to just call Super::UpdateRotation(DeltaTime), but we'll see. */
	Super::UpdateRotation(DeltaTime);
	/*
	// Calculate Delta to be applied on ViewRotation
	FRotator DeltaRot(RotationInput);

	FRotator ViewRotation = GetControlRotation();

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
	}

	AActor* ViewTarget = GetViewTarget();
	if (!PlayerCameraManager || !ViewTarget || !ViewTarget->HasActiveCameraComponent() || ViewTarget->HasActivePawnControlCameraComponent())
	{
		if (IsLocalPlayerController() && GEngine->HMDDevice.IsValid() && GEngine->HMDDevice->IsHeadTrackingAllowed())
		{
			GEngine->HMDDevice->ApplyHmdRotation(this, ViewRotation);
		}
	}

	SetControlRotation(ViewRotation);

	APawn* const P = GetPawnOrSpectator();
	if (P)
	{
		P->FaceRotation(ViewRotation, DeltaTime);
	}
	*/
}

void ASolPlayerController::PawnPendingDestroy(APawn* P)
{
	AbortSelectWeapon();
	
	// TODO: Make camera follow corpse.
	FVector LastDeathLocation = P->GetActorLocation();
	FVector CameraLocation = LastDeathLocation + FVector(0.f, 0.f, 50.f);
	FRotator LastDeathRotation = P->GetActorRotation();
	FRotator CameraRotation(-45.0f, LastDeathRotation.Yaw, 0.0f);
	FindDeathCameraSpot(CameraLocation, CameraRotation);
	
	Super::PawnPendingDestroy(P);

	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

void ASolPlayerController::ClientSetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation)
{
	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

// Taken mostly from ShooterGame.
bool ASolPlayerController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector PawnLocation = GetPawn()->GetActorLocation();
	FRotator ViewDir = GetControlRotation();
	ViewDir.Pitch = -45.0f;

	const float YawOffsets[] = { 0.0f, -180.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f };
	const float CameraOffset = 400.f;
	FCollisionQueryParams TraceParams(TEXT("DeathCamera"), true, GetPawn());

	FHitResult HitResult;
	for (int32 i = 0; i < UE_ARRAY_COUNT(YawOffsets); i++)
	{
		FRotator CameraDir = ViewDir;
		CameraDir.Yaw += YawOffsets[i];
		CameraDir.Normalize();

		const FVector TestLocation = PawnLocation - CameraDir.Vector() * CameraOffset;

		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(HitResult, PawnLocation, TestLocation, ECC_Camera, TraceParams);

		if (!bBlocked)
		{
			CameraLocation = TestLocation;
			CameraRotation = CameraDir;
			return true;
		}
	}

	return false;
}

void ASolPlayerController::BeginWaiting()
{
	ChangeState(NAME_Spectating);
	//EndPlayingState();
	//BeginSpectatingState();
}

////////////////////////////////
// Input Controls
////////////////////////////////

void ASolPlayerController::OnFire()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		if (bSelectingWeapon)
		{
			FinishSelectWeapon(WeapSelectIndex);
		}
		else
		{
			Char->StartFire();
		}
	}
	else
	{
		ServerRestartPlayer();
	}
}

void ASolPlayerController::OnStopFire()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->StopFire();
	}
}

void ASolPlayerController::OnAim()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		if (bSelectingWeapon)
		{
			AbortSelectWeapon();
		}
		else
		{
			Char->StartAim();
		}
	}
}

void ASolPlayerController::OnStopAim()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->StopAim();
	}
}

void ASolPlayerController::OnUse()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->OnStartUse();
	}
}

void ASolPlayerController::OnStopUse()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->OnStopUse();
	}
}

void ASolPlayerController::OnJump()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->Jump();
	}
	else if (GetSolSpectatorPawn())
	{
		// We need a proper way to make the pawn move up while this is held down.
		//GetSolSpectatorPawn()->MoveUp_World(1.0f);
	}
}

void ASolPlayerController::OnStopJump()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->StopJumping();
	}
}

void ASolPlayerController::OnCrouch()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->Crouch();
	}
	else if (GetSolSpectatorPawn())
	{
		// We need a proper way to make the pawn move down while this is held down.
		GetSolSpectatorPawn()->MoveUp_World(-1.0f);
	}
}

void ASolPlayerController::OnStopCrouch()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->UnCrouch();
	}
}

void ASolPlayerController::OnSprint()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->StartSprint();
	}
}

void ASolPlayerController::OnStopSprint()
{
	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->StopSprint();
	}
}

void ASolPlayerController::OnOpenInventory()
{
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->ToggleInventory();
	}
}

void ASolPlayerController::OnToggleInGameMenu()
{
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->ToggleInGameMenu();
	}
}

void ASolPlayerController::OnToggleScoreboard()
{
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->ToggleScoreboard();
	}
}

void ASolPlayerController::OnShowScoreboard()
{
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->ShowScoreboard(true);
	}
}

void ASolPlayerController::OnHideScoreboard()
{
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->ShowScoreboard(false);
	}
}

void ASolPlayerController::OnPreviousWeapon()
{
	ASolHUD* SolHUD = GetSolHUD();
	ASolCharacter* Char = GetSolCharacter();
	if (Char && SolHUD)
	{
		if (!bSelectingWeapon)
		{
			BeginSelectWeapon();
		}
		IncrementWeapSelectIndex(-1);
	}
}

void ASolPlayerController::OnNextWeapon()
{
	ASolHUD* SolHUD = GetSolHUD();
	ASolCharacter* Char = GetSolCharacter();
	if (Char && SolHUD)
	{
		if (!bSelectingWeapon)
		{
			BeginSelectWeapon();
		}
		IncrementWeapSelectIndex(1);
	}
}

void ASolPlayerController::BeginSelectWeapon()
{
	bSelectingWeapon = true;
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->SetShowWeaponList(true);
	}
	WeapSelectIndex = 0;
}

void ASolPlayerController::AbortSelectWeapon()
{
	bSelectingWeapon = false;
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->SetShowWeaponList(false);
	}
	WeapSelectIndex = 0;
}

void ASolPlayerController::FinishSelectWeapon(int32 DeltaIndex)
{
	bSelectingWeapon = false;
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->SetShowWeaponList(false);
	}

	ASolCharacter* Char = GetSolCharacter();
	if (Char)
	{
		Char->EquipWeaponByDeltaIndex(DeltaIndex);
	}

	WeapSelectIndex = 0;
}

void ASolPlayerController::IncrementWeapSelectIndex(int32 Amount)
{
	WeapSelectIndex += Amount;
	ASolHUD* SolHUD = GetSolHUD();
	if (SolHUD)
	{
		SolHUD->SetWeapSelectIndex(WeapSelectIndex);
	}
}