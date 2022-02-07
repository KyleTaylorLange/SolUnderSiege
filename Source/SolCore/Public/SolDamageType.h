// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/DamageType.h"
#include "SolDamageType.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FSolDamageEvent : public FDamageEvent
{
	GENERATED_USTRUCT_BODY()

	/** ID for this class. NOTE this must be unique for all damage events. */
	static const int32 ClassID = 4;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSolPointDamageEvent : public FPointDamageEvent
{
	GENERATED_USTRUCT_BODY()

	/** ID for this class. NOTE this must be unique for all damage events. */
	static const int32 ClassID = 5;
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FSolRadialDamageEvent : public FRadialDamageEvent
{
	GENERATED_USTRUCT_BODY()

	/** ID for this class. NOTE this must be unique for all damage events. */
	static const int32 ClassID = 6;
};

/**
 * 
 */
UCLASS()
class SOLCORE_API USolDamageType : public UDamageType
{
	GENERATED_UCLASS_BODY()

};
