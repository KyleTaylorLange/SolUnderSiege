// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Ammo_D9Mag.h"
#include "Weap_Magnum.h"

AWeap_Magnum::AWeap_Magnum(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_Magnum", "WeaponName", "Magnum");

	DefaultAmmoClass = AAmmo_D9Mag::StaticClass();

	FireMode.SetNum(1);
	FireMode[0].ShotDamage = 27.5f;
	FireMode[0].TimeBetweenShots = 0.25f;
	FireMode[0].ShotsPerBurst = 1;
	FireMode[0].AmmoPerShot = 50;
	FireMode[0].BulletProps = FBulletProperties::FBulletProperties(FLinearColor(0.75f, 0.20f, 0.95f), 9, 350);
	RecoilPerShot = 22.5f;
	SpreadRange = 75.f;
	WeaponOffset = FVector(20.f, 10.f, -10.f);
	BaseAIRating = 0.25;
}
