// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "KineticProjectile.h"

AKineticProjectile::AKineticProjectile(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bDetonatesOnImpact = true;
}
