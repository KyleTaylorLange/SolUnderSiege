// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Crate.h"

ACrate::ACrate(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("StaticMesh"));
	//Mesh = SetSimulatePhysics(true);
	Mesh->SetupAttachment(GetRootComponent());
}



