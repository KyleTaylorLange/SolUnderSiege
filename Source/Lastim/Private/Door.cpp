// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Door.h"

//////////////////////////////////////////////////////////////////////////
// ADoor

ADoor::ADoor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	OverlayBox = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, "Overlay_Box");
	OverlayBox->OnComponentBeginOverlap.AddDynamic(this, &ADoor::OnOverlapBegin);
	OverlayBox->OnComponentEndOverlap.AddDynamic(this, &ADoor::OnOverlapEnd);
	OverlayBox->SetBoxExtent(FVector(125.f, 125.f, 125.f));
	RootComponent = OverlayBox;

	DoorMeshes.Add(ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "DoorL"));
	DoorMeshes.Add(ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "DoorR"));

	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "DoorX");

	OpenPositions.Add(FVector(150.f, 0.f, 0.f));
	OpenPositions.Add(FVector(-150.f, 0.f, 0.f));
	ClosedPositions.Add(FVector(0.f, 0.f, 0.f));
	ClosedPositions.Add(FVector(0.f, 0.f, 0.f));
	
	OpenDuration = 1.f;
	CloseDuration = 1.f;
	SetCanBeDamaged(false);
	bReplicates = true;
}

void ADoor::OnOverlapBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	OpenDoor();
}

void ADoor::OnOverlapEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CloseDoor();
}

void ADoor::OpenDoor()
{
	Mesh->SetRelativeLocation(OpenPositions[0]);
	for (int32 i = 0; i < OwnedDoors.Num(); i++)
	{
		OwnedDoors[i]->OpenDoor();
	}
	DoorMeshes[0]->SetRelativeLocation(OpenPositions[0]);
	DoorMeshes[1]->SetRelativeLocation(OpenPositions[1]);
}

void ADoor::CloseDoor()
{
	Mesh->SetRelativeLocation(ClosedPositions[0]);
	for (int32 i = 0; i < OwnedDoors.Num(); i++)
	{
		OwnedDoors[i]->CloseDoor();
	}
	DoorMeshes[0]->SetRelativeLocation(ClosedPositions[0]);
	DoorMeshes[1]->SetRelativeLocation(ClosedPositions[1]);
}


