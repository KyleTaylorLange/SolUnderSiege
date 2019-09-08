// Copyright Kyle Taylor Lange

#pragma once

#include "LastimPlayerState.h"
#include "PlayerState_Assassination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API APlayerState_Assassination : public ALastimPlayerState
{
	GENERATED_UCLASS_BODY()

public:

	//APlayerState_Assassination(const FObjectInitializer& ObjectInitializer);

	/* List of other players that we can assassinate for points.*/
	TArray<APlayerState_Assassination*> AllowedKills;
};
