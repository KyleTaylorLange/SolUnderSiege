// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimCharacter.h"
#include "LastimPlayerCameraManager.h"

ALastimPlayerCameraManager::ALastimPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NormalFOV = 90.0f;
	AimingFOV = 70.0f;
	//ViewPitchMin = -87.0f;
	//ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void ALastimPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	ALastimCharacter* MyPawn = PCOwner ? Cast<ALastimCharacter>(PCOwner->GetPawn()) : NULL;
	/**
	if (MyPawn && MyPawn->IsFirstPerson())
	{
		const float TargetFOV = MyPawn->IsAiming() ? AimingFOV : NormalFOV;
		DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
	}
	**/

	Super::UpdateCamera(DeltaTime);

	if (MyPawn && MyPawn->IsFirstPerson())
	{
		MyPawn->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());
	}
}

