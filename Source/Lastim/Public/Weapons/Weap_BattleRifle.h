// Copyright Kyle Taylor Lange

#pragma once

#include "Firearm.h"
#include "Weap_BattleRifle.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AWeap_BattleRifle : public AFirearm
{
	GENERATED_BODY()
	
public:

	AWeap_BattleRifle(const FObjectInitializer& ObjectInitializer);

	/* Can the weapon attach a grenade? */
	UPROPERTY(BlueprintReadWrite, Category = Firearm)
	bool bCanAttachGrenade;
	
};
