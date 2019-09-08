// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimObjectivePoint.h"


// Sets default values
ALastimObjectivePoint::ALastimObjectivePoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	/* We need some component to place it in the level. */
	/*
	USphereComponent* SphereComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	SphereComp->InitSphereRadius(25.0f);
	RootComponent = SphereComp;*/

#if WITH_EDITORONLY_DATA
	if (GetSpriteComponent() != nullptr)
	{
		//
	}
#endif // WITH_EDITORONLY_DATA
}

