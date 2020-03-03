// Copyright Kyle Taylor Lange

#pragma once

#include "Projectile.h"
#include "KineticProjectile.generated.h"

/**
* KINETIC PROJECTILE
*  A projectile that deals damage upon impacting a target.
*/
UCLASS()
class LASTIM_API AKineticProjectile : public AProjectile
{
	GENERATED_BODY()

public:

	AKineticProjectile(const FObjectInitializer& ObjectInitializer);
};
