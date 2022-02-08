// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Ammo_Rifle.h"
#include "Weap_SMG.h"

AWeap_SMG::AWeap_SMG(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_SMG", "WeaponName", "SMG");

	DefaultAmmoClass.SetNum(1);
	DefaultAmmoClass[0] = AAmmo_Rifle::StaticClass();

	FireMode.SetNum(2);
	FireMode[0].ShotDamage = 30.f;
	FireMode[0].TimeBetweenShots = 0.125f;
	FireMode[0].ShotsPerBurst = 0;
	FireMode[0].BulletProps = FBulletProperties(FLinearColor(0.25f, 0.95f, 0.5f), 9, 450);
	FireMode[0].AmmoPerShot = 0.625f;

	FireMode[1] = FireMode[0];
	FireMode[1].BulletProps = FBulletProperties(FLinearColor(0.25f, 0.95f, 0.5f), 9, 450);
	FireMode[1].ShotsPerBurst = 1;

	MaxFireModes = 2;
	RecoilPerShot = 10;
	SpreadRange = 75.f;
	MeshOffset = FVector(10.f, 10.f, -10.f);
	BaseAIRating = 0.5f;
}


