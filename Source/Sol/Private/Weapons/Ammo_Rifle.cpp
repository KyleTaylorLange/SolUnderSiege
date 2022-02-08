// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Ammo_Rifle.h"

AAmmo_Rifle::AAmmo_Rifle(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo_Rifle", "AmmoName", "Rifle Cartridge");

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelMesh(TEXT("/Game/Meshes/Weapons/SK_RifleCartridge.SK_RifleCartridge"));
	
	if (PickupMesh)
	{
		PickupMesh->DestroyComponent();
		PickupMesh = nullptr;
	}
	USkeletalMeshComponent* Mesh3PSkel = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("AmmoSkelMesh"));
	Mesh3PSkel->SetSkeletalMesh(SkelMesh.Object);
	PickupMesh = Mesh3PSkel;
	Mesh3PSkel->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	PickupMesh->SetVisibility(false); // TEMP. TODO: Properly handle mesh visibility (when to hide and show).
	
	AmmoCount = 3000;
	MaxAmmo = 3000;

	AmmoTags.Add("Rifle");
}


