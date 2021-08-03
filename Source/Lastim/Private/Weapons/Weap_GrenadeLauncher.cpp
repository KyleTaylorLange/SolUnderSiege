// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Weap_GrenadeLauncher.h"

AWeap_GrenadeLauncher::AWeap_GrenadeLauncher(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_GrenadeLauncher", "WeaponName", "Grenade Launcher");

	FireMode.SetNum(1);
	FireMode[0].TimeBetweenShots = 0.75f;
	RecoilPerShot = 11.25;
	SpreadRange = 100.f;
	BaseAIRating = 0.7f;
	MeshOffset = FVector(10.f, 10.f, -10.f);
}


