// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Weap_BattleRifle.h"
#include "Ammo_Rifle.h"

AWeap_BattleRifle::AWeap_BattleRifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weap_BattleRifle", "WeaponName", "Battle Rifle");
	
	DefaultAmmoClass.SetNum(1);
	DefaultAmmoClass[0] = AAmmo_Rifle::StaticClass();

	MaxFireModes = 2;
	FireMode.SetNum(2);
	FireMode[0].ShotDamage = 60.f;
	FireMode[0].TimeBetweenShots = 0.2f;
	FireMode[0].ShotsPerBurst = 1;
	FireMode[0].BulletProps = FBulletProperties(FLinearColor(0.80f, 0.15f, 0.95f), 10, 1000);
	FireMode[0].AmmoPerShot = 2;

	// TODO: Make me launch a grenade instead.
	FireMode[1].ShotDamage = 0.f;
	FireMode[1].TimeBetweenShots = 0.75f;
	FireMode[1].ShotsPerBurst = 1;
	FireMode[1].AmmoPerShot = 0;

	ProjectileClass.SetNum(2);
	ProjectileClass[0] = ABullet::StaticClass();
	ProjectileClass[1] = ABullet::StaticClass(); // GRENADE

	RecoilPerShot = 18.f;
	SpreadRange = 200.f;
	MeshOffset = FVector(10.f, 10.f, -10.f);
	BaseAIRating = 0.52f;
	bCanAttachGrenade = true;
}


