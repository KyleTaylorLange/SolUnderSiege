// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Weap_SHZRifle.h"
#include "Ammo_SHZ.h"

AWeap_SHZRifle::AWeap_SHZRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_SHZRifle", "WeaponName", "SHZ Rifle");

	DefaultAmmoClass = AAmmo_SHZ::StaticClass();

	FireMode.SetNum(2);
	FireMode[0].ShotDamage = 35.f;
	FireMode[0].TimeBetweenShots = 0.125f;
	FireMode[0].ShotsPerBurst = 0; //3;
	FireMode[0].BulletProps = FBulletProperties::FBulletProperties(FLinearColor(0.25f, 0.95f, 0.25f), 9, 900);
	FireMode[0].AmmoPerShot = 75;

	FireMode[1] = FireMode[0];
	FireMode[1].ShotsPerBurst = 1; //3;

	MaxFireModes = 2;
	RecoilPerShot = 11;
	SpreadRange = 100.f;
	WeaponOffset = FVector(10.f, 10.f, -10.f);
	BaseAIRating = 0.65f;
}


