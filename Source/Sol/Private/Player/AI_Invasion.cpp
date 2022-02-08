// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "AI_Invasion.h"

bool AAI_Invasion::IsEnemy(APawn* InPawn)
{
	if (InPawn == nullptr || InPawn == GetPawn())
	{
		return false;
	}

	if (InPawn->GetController())
	{
		AAI_Invasion* FellowDrone = Cast<AAI_Invasion>(InPawn->GetController());
		if (FellowDrone != nullptr)
		{
			return false;
		}
	}

	return true;
}


