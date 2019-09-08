// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Ammo_D9Mag.h"

AAmmo_D9Mag::AAmmo_D9Mag(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo_D9Mag", "AmmoName", "Recharging Pistol Cartridge");
	
	AmmoCount = 600;
	MaxAmmo = 600;
	RechargeRateBracket.Add(FVector2D(12.f, 120));
	RechargeRateBracket.Add(FVector2D(6.f, 240));
	RechargeRateBracket.Add(FVector2D(4.f, 360));
	RechargeRateBracket.Add(FVector2D(3.f, 480));
	RechargeRateBracket.Add(FVector2D(2.4f, 600));

	RechargeAmount = 6;

	AmmoTags.Add("D9");
}


