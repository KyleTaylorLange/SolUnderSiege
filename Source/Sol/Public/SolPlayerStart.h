// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/PlayerStart.h"
#include "UObject/UnrealType.h"
#include "SolPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API ASolPlayerStart : public APlayerStart
{
	GENERATED_UCLASS_BODY()

public:

	/** Team number for this player spawn. Zero is no team. */
	UPROPERTY(EditInstanceOnly, Category = PlayerStart)
	int8 SpawnTeam;

	virtual void PostLoad() override;
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual FColor GetDrawColor();
#endif // WITH_EDITORONLY_DATA
};