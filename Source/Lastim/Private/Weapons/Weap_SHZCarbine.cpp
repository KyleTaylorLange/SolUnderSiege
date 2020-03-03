// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Weap_SHZCarbine.h"

AWeap_SHZCarbine::AWeap_SHZCarbine(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_SHZCarbine", "WeaponName", "SHZ Carbine");

	//FireMode.SetNum(2);
	FireMode[0].ShotDamage = 37.5f; //40.f
	FireMode[0].TimeBetweenShots = 0.12f; //(90% of base rifle's fire rate)
	FireMode[0].ShotsPerBurst = 0;
	FireMode[0].BulletProps = FBulletProperties::FBulletProperties(FLinearColor(0.25f, 0.95f, 0.25f), 9, 750);
	FireMode[0].AmmoPerShot = 75;

	FireMode[1] = FireMode[0];
	FireMode[1].BulletProps = FBulletProperties::FBulletProperties(FLinearColor(0.25f, 0.95f, 0.25f), 9, 750);
	FireMode[1].ShotsPerBurst = 1;

	SpreadRange = 90.f;
	RecoilPerShot = 12.5;
	BaseAIRating = 0.62f;
}


