// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Weap_SHZCarbine.h"

AWeap_SHZCarbine::AWeap_SHZCarbine(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_SHZCarbine", "WeaponName", "SHZ Carbine");

	FireMode[0].ShotDamage = 37.5f; //40.f
	FireMode[0].TimeBetweenShots = 0.21875f; // (87.5% of base rifle's fire rate) //0.12f; //(90% of base rifle's fire rate)
	FireMode[0].ShotsPerBurst = 0;
	FireMode[0].BulletProps = FBulletProperties(FLinearColor(0.25f, 0.95f, 0.25f), 9, 750);
	FireMode[0].AmmoPerShot = 0.84f;

	FireMode[1].ShotDamage = FireMode[0].ShotDamage;
	FireMode[1].TimeBetweenShots = FireMode[0].TimeBetweenShots;
	FireMode[1].ShotsPerBurst = 1;
	FireMode[1].BulletProps = FireMode[0].BulletProps;
	FireMode[1].AmmoPerShot = FireMode[0].AmmoPerShot;

	SpreadRange = 90.f;
	RecoilPerShot = 12.5;
	BaseAIRating = 0.62f;
}


