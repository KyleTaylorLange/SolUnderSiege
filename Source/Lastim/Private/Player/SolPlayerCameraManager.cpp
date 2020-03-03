// Copyright Kyle Taylor Lange

#include "Lastim.h"
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

