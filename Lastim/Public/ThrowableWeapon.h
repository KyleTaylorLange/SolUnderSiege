// Copyright Kyle Taylor Lange

#pragma once

#include "LastimWeapon.h"
#include "ThrowableWeapon.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AThrowableWeapon : public ALastimWeapon
{
	GENERATED_UCLASS_BODY()

public:
	
	virtual void StartFire() override;
	
	virtual void StopFire() override;

	virtual void DestroyAfterUse();

protected:

	bool bGrenadeArmed;
};
