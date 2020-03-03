// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "InventoryBag.h"

AInventoryBag::AInventoryBag(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkelMesh(TEXT("/Game/Character/UE4_Mannequin/Mesh/SK_Mannequin_1PArms.SK_Mannequin_1PArms"));

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("BagMesh"));
	Mesh3P->SetSkeletalMesh(SkelMesh.Object);
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	bReplicates = true;
}

UMeshComponent* AInventoryBag::GetPickupMesh()
{
	if (Mesh3P)
	{
		USkeletalMeshComponent* OutMesh = GetClass()->GetDefaultObject<AInventoryBag>()->Mesh3P;
		return OutMesh;
	}
	else
	{
		return Super::GetPickupMesh();
	}
}