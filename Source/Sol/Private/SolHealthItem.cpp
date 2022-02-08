// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolCharacter.h"
#include "SolHealthItem.h"

ASolHealthItem::ASolHealthItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	HealthNumRegained = 12.5f;
	HealthPctRegained = 0.0f;
}

void ASolHealthItem::StartFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (MyPawn)
	{
		MyPawn->SetHealth(MyPawn->GetHealth() + HealthNumRegained);
	}
}

void ASolHealthItem::StopFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerStopFire();
	}
}
