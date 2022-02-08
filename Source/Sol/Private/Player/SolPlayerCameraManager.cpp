// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolCharacter.h"
#include "SolPlayerCameraManager.h"

ASolPlayerCameraManager::ASolPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NormalFOV = 90.0f;
	AimingFOV = 75.0f;
	SprintingFOV = 100.0f;
	//ViewPitchMin = -87.0f;
	//ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
	bLimitViewRotation = false;
}

void ASolPlayerCameraManager::ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
	//SCOPE_CYCLE_COUNTER(STAT_Camera_ProcessViewRotation);
	for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ModifierIdx++)
	{
		if (ModifierList[ModifierIdx] != NULL &&
			!ModifierList[ModifierIdx]->IsDisabled())
		{
			if (ModifierList[ModifierIdx]->ProcessViewRotation(ViewTarget.Target, DeltaTime, OutViewRotation, OutDeltaRot))
			{
				break;
			}
		}
	}

	// Add Delta Rotation
	//OutViewRotation = (FQuat(OutDeltaRot) * FQuat(OutViewRotation)).Rotator();
	OutViewRotation += OutDeltaRot;
	OutDeltaRot = FRotator::ZeroRotator;

	// NOTE: Had to comment out some VR stuff to get it to compile. May be important to recreate if we do VR.
	if (!bLimitViewRotation) //(GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed())
	{
		// With the HMD devices, we can't limit the view pitch, because it's bound to the player's head.  A simple normalization will suffice
		OutViewRotation.Normalize();
	}
	else
	{
		// Limit Player View Axes
		LimitViewPitch(OutViewRotation, ViewPitchMin, ViewPitchMax);
		LimitViewYaw(OutViewRotation, ViewYawMin, ViewYawMax);
		LimitViewRoll(OutViewRotation, ViewRollMin, ViewRollMax);
	}
}

void ASolPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	ASolCharacter* MyPawn = PCOwner ? Cast<ASolCharacter>(PCOwner->GetPawn()) : NULL;

	Super::UpdateCamera(DeltaTime);

	if (MyPawn && MyPawn->IsFirstPerson())
	{
		
		MyPawn->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());
	}
}
