// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "ObjectivePoint.h"

AObjectivePoint::AObjectivePoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

#if WITH_EDITORONLY_DATA
	if (GetSpriteComponent() != nullptr)
	{
		//
	}
#endif // WITH_EDITORONLY_DATA
}

