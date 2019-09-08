// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Ammo_BRSlugs.h"

AAmmo_BRSlugs::AAmmo_BRSlugs(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo_BRSlugs", "AmmoName", "BR Slugs");
	
	AmmoCount = 48;
	MaxAmmo = 60;
	bIsLooseAmmo = true;

	AmmoTags.Add("BR");
}

FString AAmmo_BRSlugs::GetDisplayName() const
{
	return FString::Printf(TEXT("%s (%d)"), *DisplayName.ToString(), AmmoCount);
		//Super::GetDisplayName();
}
