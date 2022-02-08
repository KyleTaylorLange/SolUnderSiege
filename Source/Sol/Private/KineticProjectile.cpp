// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "KineticProjectile.h"

AKineticProjectile::AKineticProjectile(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bDetonatesOnImpact = true;
}
