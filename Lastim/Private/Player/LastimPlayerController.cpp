// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimCharacter.h"
#include "LastimSpectatorPawn.h"
#include "LastimPlayerCameraManager.h"
#include "LastimPlayerState.h"
#include "LastimHUD.h"
#include "LastimPlayerController.h"

ALastimPlayerController::ALastimPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ALastimPlayerCameraManager::StaticClass();

	bSelectingWeapon = false;
	WeapSelectIndex = 0;
}

void ALastimPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Fire", IE_Pressed, this, &ALastimPlayerController::OnFire);
	InputComponent->BindAction("Fire", IE_Released, this, &ALastimPlayerController::OnStopFire);

	InputComponent->BindAction("Aim", IE_Pressed, this, &ALastimPlayerController::OnAim);
	InputComponent->BindAction("Aim", IE_Released, this, &ALastimPlayerController::OnStopAim);

	InputComponent->BindAction("Use", IE_Pressed, this, &ALastimPlayerController::OnUse);
	InputComponent->BindAction("Use", IE_Released, this, &ALastimPlayerController::OnStopUse);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ALastimPlayerController::OnJump);
	InputComponent->BindAction("Jump", IE_Released, this, &ALastimPlayerController::OnStopJump);

	InputComponent->BindAction("Crouch", IE_Pressed, this, &ALastimPlayerController::OnCrouch);
	InputComponent->BindAction("Crouch", IE_Released, this, &ALastimPlayerController::OnStopCrouch);

	InputComponent->BindAction("Sprint", IE_Pressed, this, &ALastimPlayerController::OnSprint);
	InputComponent->BindAction("Sprint", IE_Released, this, &ALastimPlayerController::OnStopSprint);

	InputComponent->BindAction("OpenInventory", IE_Pressed, this, &ALastimPlayerController::OnOpenInventory);

	InputComponent->BindAction("ToggleInGameMenu", IE_Pressed, this, &ALastimPlayerController::OnToggleInGameMenu);

	InputComponent->BindAction("ToggleScoreboard", IE_Pressed, this, &ALastimPlayerController::OnToggleScoreboard);

	InputComponent->BindAction("ShowScoreboard", IE_Pressed, this, &ALastimPlayerController::OnShowScoreboard);
	InputComponent->BindAction("ShowScoreboard", IE_Released, this, &ALastimPlayerController::OnHideScoreboard);

	InputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &ALastimPlayerController::OnPreviousWeapon);
	InputComponent->BindAction("NextWeapon", IE_Pressed, this, &ALastimPlayerController::OnNextWeapon);
}

ALastimCharacter* ALastimPlayerController::GetLastimCharacter() const
{
	return Cast<ALastimCharacter>(GetCharacter());
}

ALastimSpectatorPawn* ALastimPlayerController::GetLastimSpectatorPawn() const
{
	return Cast<ALastimSpectatorPawn>(GetSpectatorPawn());
}

ALastimHUD* ALastimPlayerController::GetLastimHUD() const
{
	return Cast<ALastimHUD>(GetHUD());
}

void ALastimPlayerController::OnDeathMessage(class ALastimPlayerState* KillerPlayerState, class ALastimPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->AddDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
	}
}

bool ALastimPlayerController::CanRestartPlayer()
{
	ALastimPlayerState* PS = Cast<ALastimPlayerState>(PlayerState);
	if (PS)
	{
		return Super::CanRestartPlayer() && PS->RespawnTime <= 0.0f;
	}
	return false;
}

void ALastimPlayerController::UpdateRotation(float DeltaTime)
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

void ALastimPlayerController::PawnPendingDestroy(APawn* P)
{
	AbortSelectWeapon();
	
	FVector LastDeathLocation = P->GetActorLocation();
	FVector CameraLocation = LastDeathLocation + FVector(0.f, 0.f, 100.f);
	FRotator LastDeathRotation = P->GetActorRotation();
	FRotator CameraRotation(-45.0f, LastDeathRotation.Yaw, 0.0f);
	FindDeathCameraSpot(CameraLocation, CameraRotation);
	
	Super::PawnPendingDestroy(P);

	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

void ALastimPlayerController::ClientSetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation)
{
	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

bool ALastimPlayerController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector PawnLocation = GetPawn()->GetActorLocation();
	FRotator ViewDir = GetControlRotation();
	ViewDir.Pitch = -45.0f;

	const float YawOffsets[] = { 0.0f, -180.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f };
	const float CameraOffset = 400.f;
	FCollisionQueryParams TraceParams(TEXT("DeathCamera"), true, GetPawn());

	FHitResult HitResult;
	for (int32 i = 0; i < ARRAY_COUNT(YawOffsets); i++)
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

void ALastimPlayerController::BeginWaiting()
{
	ChangeState(NAME_Spectating);
	//EndPlayingState();
	//BeginSpectatingState();
}

////////////////////////////////
// Input Controls
////////////////////////////////

void ALastimPlayerController::OnFire()
{
	ALastimCharacter* Char = GetLastimCharacter();
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

void ALastimPlayerController::OnStopFire()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->StopFire();
	}
}

void ALastimPlayerController::OnAim()
{
	ALastimCharacter* Char = GetLastimCharacter();
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

void ALastimPlayerController::OnStopAim()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->StopAim();
	}
}

void ALastimPlayerController::OnUse()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->OnStartUse();
	}
}

void ALastimPlayerController::OnStopUse()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->OnStopUse();
	}
}

void ALastimPlayerController::OnJump()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->Jump();
	}
	else if (GetLastimSpectatorPawn())
	{
		GetLastimSpectatorPawn()->MoveUp_World(1.0f);
	}
}

void ALastimPlayerController::OnStopJump()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->StopJumping();
	}
}

void ALastimPlayerController::OnCrouch()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->Crouch();
	}
	else if (GetLastimSpectatorPawn())
	{
		GetLastimSpectatorPawn()->MoveUp_World(-1.0f);
	}
}

void ALastimPlayerController::OnStopCrouch()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->UnCrouch();
	}
}

void ALastimPlayerController::OnSprint()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->StartSprint();
	}
}

void ALastimPlayerController::OnStopSprint()
{
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->StopSprint();
	}
}

void ALastimPlayerController::OnOpenInventory()
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->ToggleInventory();
	}
}

void ALastimPlayerController::OnToggleInGameMenu()
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->ToggleInGameMenu();
	}
}

void ALastimPlayerController::OnToggleScoreboard()
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->ToggleScoreboard();
	}
}

void ALastimPlayerController::OnShowScoreboard()
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->ShowScoreboard(true);
	}
}

void ALastimPlayerController::OnHideScoreboard()
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->ShowScoreboard(false);
	}
}

void ALastimPlayerController::OnPreviousWeapon()
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char && LastimHUD)
	{
		if (!bSelectingWeapon)
		{
			BeginSelectWeapon();
		}
		IncrementWeapSelectIndex(-1);
	}
}

void ALastimPlayerController::OnNextWeapon()
{
	ALastimHUD* LastimHUD = GetLastimHUD();
	ALastimCharacter* Char = GetLastimCharacter();
	if (Char && LastimHUD)
	{
		if (!bSelectingWeapon)
		{
			BeginSelectWeapon();
		}
		IncrementWeapSelectIndex(1);
	}
}

void ALastimPlayerController::BeginSelectWeapon()
{
	bSelectingWeapon = true;
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->SetShowWeaponList(true);
	}
	WeapSelectIndex = 0;
}

void ALastimPlayerController::AbortSelectWeapon()
{
	bSelectingWeapon = false;
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->SetShowWeaponList(false);
	}
	WeapSelectIndex = 0;
}

void ALastimPlayerController::FinishSelectWeapon(int32 DeltaIndex)
{
	bSelectingWeapon = false;
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->SetShowWeaponList(false);
	}

	ALastimCharacter* Char = GetLastimCharacter();
	if (Char)
	{
		Char->EquipWeaponByDeltaIndex(DeltaIndex);
	}

	WeapSelectIndex = 0;
}

void ALastimPlayerController::IncrementWeapSelectIndex(int32 Amount)
{
	WeapSelectIndex += Amount;
	ALastimHUD* LastimHUD = GetLastimHUD();
	if (LastimHUD)
	{
		LastimHUD->SetWeapSelectIndex(WeapSelectIndex);
	}
}