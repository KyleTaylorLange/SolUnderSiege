// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Ammo_Pistol.h"
#include "Weap_Tsume.h"

AWeap_Tsume::AWeap_Tsume(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_Tsume", "WeaponName", "Tsume Pistol");

	DefaultAmmoClass.SetNum(1);
	DefaultAmmoClass[0] = AAmmo_Pistol::StaticClass();

	FireMode.SetNum(1);
	FireMode[0].ShotDamage = 34.0f;
	FireMode[0].TimeBetweenShots = 0.2875f;
	FireMode[0].ShotsPerBurst = 1;
	FireMode[0].BulletProps = FBulletProperties(FLinearColor(0.25f, 0.55f, 0.89f), 11, 240);
	FireMode[0].AmmoPerShot = .5625f;

	FirearmConfig.AimSpeed = 0.3f;
	RecoilPerShot = 25;
	SpreadRange = 60.f;
	MeshOffset = FVector(20.f, 10.f, -10.f);
	BaseAIRating = 0.25;
	WeaponSlotType = WeaponSlotType::Sidearm;
}
