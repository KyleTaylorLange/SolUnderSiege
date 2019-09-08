// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Weap_Blunderbuss.h"

AWeap_Blunderbuss::AWeap_Blunderbuss(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_Blunderbuss", "WeaponName", "Blunderbuss");

	MaxFireModes = 2;
	FireMode.SetNum(2);
	FireMode[0].ShotDamage = 120.f;
	FireMode[0].TimeBetweenShots = 0.35f;
	FireMode[0].ShotsPerBurst = 1;
	FireMode[0].BulletProps = FBulletProperties::FBulletProperties(FLinearColor(0.80f, 0.15f, 0.95f), 10, 750);
	FireMode[0].AmmoPerShot = 375;

	FireMode[1].ShotDamage = 30.f;
	FireMode[1].TimeBetweenShots = 0.35f;
	FireMode[1].ShotsPerBurst = 1;
	FireMode[1].BulletProps = FBulletProperties::FBulletProperties(FLinearColor(0.80f, 0.15f, 0.95f), 5, 500);
	FireMode[1].AmmoPerShot = 250;

	ProjectileCount.SetNum(2);
	ProjectileCount[0] = 1;
	ProjectileCount[1] = 4;

	ProjectileClass.SetNum(2);
	ProjectileClass[0] = ABullet::StaticClass();
	ProjectileClass[1] = ABullet::StaticClass();

	FirearmConfig.AimSpeed = 0.45f;
	SpreadRadius = 100;
	SpreadRange = 45;
	WeaponOffset = FVector(10.f, 10.f, -10.f);
	RecoilPerShot = 12.5;
	BaseAIRating = 0.45f;
	bCanAttachGrenade = false;
}


