// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "GameMode_Scenario.h"

AGameMode_Scenario::AGameMode_Scenario(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Scenario", "Scenario", "Scenario");
}
