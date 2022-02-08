// Copyright Kyle Taylor Lange

#pragma once

#include "Weapon.h"
#include "ThrowableWeapon.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AThrowableWeapon : public AWeapon
{
	GENERATED_UCLASS_BODY()

public:
	
	virtual void StartFire() override;
	
	virtual void StopFire() override;

	virtual void DestroyAfterUse();

protected:

	bool bGrenadeArmed;
};
