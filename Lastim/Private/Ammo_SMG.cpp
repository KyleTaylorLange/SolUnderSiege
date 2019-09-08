// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Ammo_SMG.h"

AAmmo_SMG::AAmmo_SMG(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo_SMG", "AmmoName", "SMG Ammo");
	
	AmmoCount = 45;
	MaxAmmo = 45;

	AmmoTags.Add("SMG");
}



