// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "SolSpectatorPawn.h"
#include "SolPlayerCameraManager.h"
#include "SolPlayerState.h"
#include "SolGameMode.h"
#include "SolHUD.h"
#include "SolLocalPlayer.h"
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

	InputComponent->BindAction("Reload", IE_Pressed, this, &ASolPlayerController::OnReload);
	InputComponent->BindAction("Reload", IE_Released, this, &ASolPlayerController::OnStopReload);

	InputComponent->BindAction("SwitchFireMode", IE_Pressed, this, &ASolPlayerController::OnSwitchFireMode);
	InputComponent->BindAction("SwitchFireMode", IE_Released, this, &ASolPlayerController::OnStopSwitchFireMode);

	InputComponent->BindAxis("MoveForward", this, &ASolPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASolPlayerController::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ASolPlayerController::MoveUp);

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

void ASolPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	USolLocalPlayer* SolLP = Cast<USolLocalPlayer>(GetLocalPlayer());
	if (SolLP)
	{
		SetPlayerColors(SolLP->GetPrimaryColor(), SolLP->GetSecondaryColor());
	}
}

void ASolPlayerController::Say(const FString& Msg)
{
	FName SayName = TEXT("Say");
	ServerSay(Msg, SayName);
}

void ASolPlayerController::TeamSay(const FString& Msg)
{
	FName TeamSayName = TEXT("TeamSay");
	ServerSay(Msg, TeamSayName);
}

bool ASolPlayerController::ServerSay_Validate(const FString& Msg, FName Type)
{
	return true;
}

void ASolPlayerController::ServerSay_Implementation(const FString& Msg, FName Type)
{
	GetWorld()->GetAuthGameMode<ASolGameMode>()->Broadcast(this, Msg, Type);
}

void ASolPlayerController::ClientSendMessage_Implementation(class ASolPlayerState* SenderPlayerState, const FString& S, FName Type)
{
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->AddChatMessage(SenderPlayerState, S, Type);
	}
}

ASolCharacter* ASolPlayerController::GetSolCharacter() const
{
	return Cast<ASolCharacter>(GetCharacter());
}

ASolSpectatorPawn* ASolPlayerController::GetSolSpectatorPawn() const
{
	return Cast<ASolSpectatorPawn>(GetSpectatorPawn());
}

void ASolPlayerController::OnDeathMessage_Implementation(class ASolPlayerState* KillerPlayerState, class ASolPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{
	ASolHUD* SolHUD = GetHUD<ASolHUD>();
	if (SolHUD)
	{
		SolHUD->AddDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
	}
}

void ASolPlayerController::SetColors(FLinearColor Primary, FLinearColor Secondary)
{
	SetPlayerColors(Primary, Secondary);
}

void ASolPlayerController::SetPlayerColors_Implementation(FLinearColor Primary, FLinearColor Secondary)
{
	ASolPlayerState* SolPS = Cast<ASolPlayerState>(PlayerState);
	if (SolPS)
	{
		SolPS->SetPrimaryColor(Primary);
		SolPS->SetSecondaryColor(Secondary);
	}
}

bool ASolPlayerController::CanRestartPlayer()
{
	ASolPlayerState* SolPS = Cast<ASolPlayerState>(PlayerState);
	if (SolPS)
	{
		return Super::CanRestartPlayer() && SolPS->RespawnTime <= 0.0f;
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
		if (IsLocalPlayerController() && GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed())
		{
			auto XRCamera = GEngine->XRSystem->GetXRCamera();
			if (XRCamera.IsValid())
			{
				XRCamera->ApplyHMDRotation(this, ViewRotation);
			}
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

	ClientSetSpectatorCamera(CameraLocation, CameraRotation);
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
	if (GetSolCharacter())
	{
		if (bSelectingWeapon)
		{
			FinishSelectWeapon(WeapSelectIndex);
		}
		else
		{
			GetSolCharacter()->StartFire();
		}
	}
	else
	{
		ServerRestartPlayer();
	}
}

void ASolPlayerController::OnStopFire()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->StopFire();
	}
}

void ASolPlayerController::OnAim()
{
	if (GetSolCharacter())
	{
		if (bSelectingWeapon)
		{
			AbortSelectWeapon();
		}
		else
		{
			GetSolCharacter()->StartAim();
		}
	}
}

void ASolPlayerController::OnStopAim()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->StopAim();
	}
}

void ASolPlayerController::OnUse()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->OnStartUse();
	}
}

void ASolPlayerController::OnStopUse()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->OnStopUse();
	}
}

void ASolPlayerController::OnReload()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->OnReload();
	}
}

void ASolPlayerController::OnStopReload()
{
	// Nothing for now.
}

void ASolPlayerController::OnSwitchFireMode()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->OnSwitchFireMode();
	}
}

void ASolPlayerController::OnStopSwitchFireMode()
{
	// Nothing for now.
}

void ASolPlayerController::MoveForward(float Value)
{
	if (Value != 0.0f && GetSolCharacter())
	{
		GetSolCharacter()->MoveForward(Value);
	}
	else if (GetSolSpectatorPawn())
	{
		GetSolSpectatorPawn()->MoveForward(Value);
	}
}

void ASolPlayerController::MoveRight(float Value)
{
	if (Value != 0.0f && GetSolCharacter())
	{
		GetSolCharacter()->MoveRight(Value);
	}
	else if (GetSolSpectatorPawn())
	{
		GetSolSpectatorPawn()->MoveRight(Value);
	}
}

void ASolPlayerController::MoveUp(float Value)
{
	if (Value != 0.0f && GetSolCharacter())
	{
		GetSolCharacter()->MoveUp(Value);
	}
	else if (GetSolSpectatorPawn())
	{
		GetSolSpectatorPawn()->MoveUp(Value);
	}
}

void ASolPlayerController::OnJump()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->Jump();
	}
	else if (GetSolSpectatorPawn())
	{
		// TODO: Properly implement having character move up when jump is held down. Use axes bind with crouch?
		//GetSolSpectatorPawn()->MoveUp_World(1.0f);
	}
}

void ASolPlayerController::OnStopJump()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->StopJumping();
	}
}

void ASolPlayerController::OnCrouch()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->Crouch();
	}
	else if (GetSolSpectatorPawn())
	{
		// TODO: Properly implement having character move up when jump is held down. Use axes bind with jump?
		//GetSolSpectatorPawn()->MoveUp_World(-1.0f);
	}
}

void ASolPlayerController::OnStopCrouch()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->UnCrouch();
	}
}

void ASolPlayerController::OnSprint()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->StartSprint();
	}
}

void ASolPlayerController::OnStopSprint()
{
	if (GetSolCharacter())
	{
		GetSolCharacter()->StopSprint();
	}
}

void ASolPlayerController::OnOpenInventory()
{
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->ToggleInventory();
	}
}

void ASolPlayerController::OnToggleInGameMenu()
{
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->ToggleInGameMenu();
	}
}

void ASolPlayerController::OnToggleScoreboard()
{
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->ToggleScoreboard();
	}
}

void ASolPlayerController::OnShowScoreboard()
{
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->ShowScoreboard(true);
	}
}

void ASolPlayerController::OnHideScoreboard()
{
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->ShowScoreboard(false);
	}
}

void ASolPlayerController::OnPreviousWeapon()
{
	if (GetSolCharacter() && GetHUD<ASolHUD>())
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
	if (GetSolCharacter() && GetHUD<ASolHUD>())
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
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->SetShowWeaponList(true);
	}
	WeapSelectIndex = 0;
}

void ASolPlayerController::AbortSelectWeapon()
{
	bSelectingWeapon = false;
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->SetShowWeaponList(false);
	}
	WeapSelectIndex = 0;
}

void ASolPlayerController::FinishSelectWeapon(int32 DeltaIndex)
{
	bSelectingWeapon = false;
	if (GetHUD<ASolHUD>())
	{
		GetHUD<ASolHUD>()->SetShowWeaponList(false);
	}

	if (GetSolCharacter())
	{
		GetSolCharacter()->EquipWeaponByDeltaIndex(DeltaIndex);
	}

	WeapSelectIndex = 0;
}

void ASolPlayerController::IncrementWeapSelectIndex(int32 Amount)
{
	WeapSelectIndex += Amount;
	ASolHUD* SolHUD = GetHUD<ASolHUD>();
	if (SolHUD)
	{
		SolHUD->SetWeapSelectIndex(WeapSelectIndex);
	}
}