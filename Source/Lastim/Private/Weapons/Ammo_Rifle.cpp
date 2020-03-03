// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Ammo_Rifle.h"

AAmmo_Rifle::AAmmo_Rifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo_Rifle", "AmmoName", "Rifle Cartridge");

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelMesh(TEXT("/Game/Meshes/Weapons/SK_RifleCartridge.SK_RifleCartridge"));
	Mesh3P->SetSkeletalMesh(SkelMesh.Object);
	
	AmmoCount = 3000;
	MaxAmmo = 3000;

	AmmoTags.Add("Rifle");
}


