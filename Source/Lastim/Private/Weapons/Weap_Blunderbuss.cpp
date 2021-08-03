// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Weap_Blunderbuss.h"

AWeap_Blunderbuss::AWeap_Blunderbuss(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_Blunderbuss", "WeaponName", "Blunderbuss");

	MaxFireModes = 2;
	FireMode.SetNum(2);
	// Heavy slug
	FireMode[0].ShotDamage = 120.f;
	FireMode[0].TimeBetweenShots = 0.45f;
	FireMode[0].ShotsPerBurst = 1;
	FireMode[0].BulletProps = FBulletProperties(FLinearColor(0.80f, 0.15f, 0.95f), 10, 750);
	FireMode[0].AmmoPerShot = 3.75f;

	// Spreadfire 
	FireMode[1].ShotDamage = 20.f;
	FireMode[1].TimeBetweenShots = 0.35f;
	FireMode[1].ShotsPerBurst = 1;
	FireMode[1].BulletProps = FBulletProperties(FLinearColor(0.80f, 0.15f, 0.95f), 5, 500);
	FireMode[1].AmmoPerShot = 2.5f;
	FireMode[1].ProjectileCount = 6;

	FirearmConfig.AimSpeed = 0.45f;
	SpreadRadius = 100;
	SpreadRange = 45;
	MeshOffset = FVector(10.f, 10.f, -10.f);
	RecoilPerShot = 12.5;
	BaseAIRating = 0.45f;
	bCanAttachGrenade = false;
}


