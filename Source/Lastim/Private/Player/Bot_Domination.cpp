// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "DominationControlPoint.h"
#include "Bot_Domination.h"

bool ABot_Domination::CheckObjective()
{
	ADominationControlPoint* DesiredCP = nullptr;
	/* Shitty code to go to a random Control Point. */
	for (TActorIterator<ADominationControlPoint> It(GetWorld()); It; ++It)
	{
		ADominationControlPoint* CP = *It;
		if (CP)
		{
			if (DesiredCP == nullptr)
			{
				DesiredCP = CP;
			}
			else if (FMath::FRand() < 0.5f)
			{
				DesiredCP = CP;
			}
		}
	}
	if (DesiredCP)
	{
		//MoveToLocation(DesiredCP->GetActorLocation(), 1, true, true, false, true);


		FAIMoveRequest MoveReq;
		//MoveReq.SetNavigationFilter(FilterClass);
		MoveReq.SetAllowPartialPath(true); //(bAllowPartialPath);
		MoveReq.SetAcceptanceRadius(10.f); //(AcceptableRadius);
		MoveReq.SetCanStrafe(false); //(bAllowStrafe);
		//MoveReq.SetStopOnOverlap(false);  //(bStopOnOverlap);
		MoveReq.SetReachTestIncludesAgentRadius(false);
		MoveReq.SetGoalLocation(DesiredCP->GetActorLocation());
		MoveTo(MoveReq);
		return true;
	}
	
	return false;
}


