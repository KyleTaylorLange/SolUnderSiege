// Copyright Kyle Taylor Lange

#pragma once

#include "Projectile.h"
#include "Grenade.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AGrenade : public AProjectile
{
	GENERATED_BODY()
	
public:

	AGrenade(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/** Time until the grenade explodes. */
	float FuseTime;

	/** Called when fuse timer ends. */
	virtual void FuseTimer();
	
};
