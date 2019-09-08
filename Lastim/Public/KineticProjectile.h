// Copyright Kyle Taylor Lange

#pragma once

#include "LastimProjectile.h"
#include "KineticProjectile.generated.h"

/**
* KINETIC PROJECTILE
*  A projectile that deals damage upon impacting a target.
*/
UCLASS()
class LASTIM_API AKineticProjectile : public ALastimProjectile
{
	GENERATED_BODY()

public:

	AKineticProjectile(const FObjectInitializer& ObjectInitializer);
};
