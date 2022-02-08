// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Weap_SHZRifle.h"
#include "Ammo_Rifle.h"

AWeap_SHZRifle::AWeap_SHZRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_SHZRifle", "WeaponName", "SHZ Rifle");

	DefaultAmmoClass.SetNum(1);
	DefaultAmmoClass[0] = AAmmo_Rifle::StaticClass();

	FireMode.SetNum(2);
	FireMode[0].ShotDamage = 40.f;
	FireMode[0].TimeBetweenShots = 0.25f; //0.125f;
	FireMode[0].ShotsPerBurst = 0; //3;
	FireMode[0].BulletProps = FBulletProperties(FLinearColor(0.25f, 0.95f, 0.25f), 9, 900);
	FireMode[0].AmmoPerShot = 0.85f;

	FireMode[1] = FireMode[0];
	FireMode[1].ShotsPerBurst = 1;

	MaxFireModes = 2;
	RecoilPerShot = 11;
	SpreadRange = 100.f;
	MeshOffset = FVector(10.f, 10.f, -10.f);
	BaseAIRating = 0.65f;
}


