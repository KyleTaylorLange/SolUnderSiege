// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "ObjectivePoint.h"

AObjectivePoint::AObjectivePoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
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

