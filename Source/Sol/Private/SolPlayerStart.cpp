// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolPlayerStart.h"

//////////////////////////////////////////////////////////////////////////
// ASolPlayerStart

ASolPlayerStart::ASolPlayerStart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SpawnTeam = 0;
	
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 92.0f);
	GetCapsuleComponent()->SetShouldUpdatePhysicsVolume(false);

#if WITH_EDITORONLY_DATA
	
	if (GetArrowComponent())
	{
		GetArrowComponent()->ArrowColor = GetDrawColor();
	}

#endif // WITH_EDITORONLY_DATA
}

void ASolPlayerStart::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITORONLY_DATA
	if (GetArrowComponent())
	{
		GetArrowComponent()->ArrowColor = GetDrawColor();
	}
#endif // WITH_EDITORONLY_DATA
}

#if WITH_EDITORONLY_DATA
void ASolPlayerStart::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GetArrowComponent())
	{
		// TODO: Debug: this always seems to be one change behind.
		// E.g. nothing happens when changing from team 0 to 3 (green),
		//      but changing to anything else then changes it to green.
		// Note: colour is correct if the start is moved - does some rendering update need to occur?
		GetArrowComponent()->ArrowColor = GetDrawColor();
	}
}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITORONLY_DATA
FColor ASolPlayerStart::GetDrawColor()
{
	if (SpawnTeam == 1)
	{
		return FColor::Red;
	}
	if (SpawnTeam == 2)
	{
		return FColor::Blue;
	}
	if (SpawnTeam == 3)
	{
		return FColor::Green;
	}
	if (SpawnTeam == 4)
	{
		return FColor::Yellow;
	}
	if (SpawnTeam == 5)
	{
		return FColor::Purple;
	}
	if (SpawnTeam == 6)
	{
		return FColor::Orange;
	}
	if (SpawnTeam == 7)
	{
		return FColor::Cyan;
	}
	if (SpawnTeam == 8)
	{
		return FColor(127, 127, 127); // Silver
	}
	if (SpawnTeam >= 9)
	{
		return FColor::Magenta;
	}
	return FColor::White;
}
#endif // WITH_EDITORONLY_DATA