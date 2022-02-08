// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Weap_D9Pistol.h"
#include "Ammo_Pistol.h"

AWeap_D9Pistol::AWeap_D9Pistol(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_D9Pistol", "WeaponName", "D9 Pistol");

	DefaultAmmoClass.SetNum(1);
	DefaultAmmoClass[0] = AAmmo_Pistol::StaticClass();

	FireMode.SetNum(1);
	FireMode[0].ShotDamage = 27.5f;
	FireMode[0].TimeBetweenShots = 0.225f;
	FireMode[0].ShotsPerBurst = 1;
	FireMode[0].BulletProps = FBulletProperties(FLinearColor(0.25f, 0.55f, 0.89f), 9, 260);
	FireMode[0].AmmoPerShot = 0.3f;

	FirearmConfig.AimSpeed = 0.3f;
	RecoilPerShot = 15;
	SpreadRange = 50.f;
	MeshOffset = FVector(20.f, 10.f, -10.f);
	BaseAIRating = 0.25;
	WeaponSlotType = WeaponSlotType::Sidearm;
}


