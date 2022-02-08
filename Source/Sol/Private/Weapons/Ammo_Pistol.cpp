// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Ammo_Pistol.h"

AAmmo_Pistol::AAmmo_Pistol(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Ammo_Pistol", "AmmoName", "Regenerating Pistol Cartridge");

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelMesh(TEXT("/Game/Meshes/Weapons/SK_RifleCartridge.SK_RifleCartridge"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> MatInstance(TEXT("/Game/Materials/MI_PistolCartridge.MI_PistolCartridge"));
	//Mesh3P->SetSkeletalMesh(SkelMesh.Object);
	// This doesnt'seem to work.
	//Mesh3P->SetMaterial(0, MatInstance.Object);

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
	PickupMesh->SetMaterial(0, MatInstance.Object);
	
	AmmoCount = 600;
	MaxAmmo = 600;
	/*RechargeRateBracket.Add(FVector2D(12.f, 120));
	RechargeRateBracket.Add(FVector2D(6.f, 240));
	RechargeRateBracket.Add(FVector2D(4.f, 360));
	RechargeRateBracket.Add(FVector2D(3.f, 480));
	RechargeRateBracket.Add(FVector2D(2.4f, 600));*/
	// The above was too slow for current gameplay where player starts w/ only one cartridge.
	// This gives the player a Tsume shot every 2.5 seconds and a D9 shot every 1.333 seconds (until half ammo).
	// Still may be too slow.
	RechargeRateBracket.Add(FVector2D(30.f, 300));
	RechargeRateBracket.Add(FVector2D(15.f, 600));

	RechargeAmount = 6;

	AmmoTags.Add("Pistol");
}


