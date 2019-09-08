// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Ammo_SHZ.h"

AAmmo_SHZ::AAmmo_SHZ(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo_SHZ", "AmmoName", "Rifle Cartridge");
	
	AmmoCount = 3000;
	MaxAmmo = 3000;

	AmmoTags.Add("SHZ");
}


