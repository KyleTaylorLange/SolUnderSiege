// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "DroppedPickup.h"

//////////////////////////////////////////////////////////////////////////
// ADroppedPickup
//   Pickup class for items dropped by the player.
//   Currently, this just sets a lifespan.
//////////////////////////////////////////////////////////////////////////

ADroppedPickup::ADroppedPickup(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void ADroppedPickup::BeginPlay()
{
	SetLifeSpan(60.f);
}

