// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "UI/SolHUD.h"
#include "HUD_LastTeamStanding.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AHUD_LastTeamStanding : public ASolHUD
{
	GENERATED_BODY()

	AHUD_LastTeamStanding(const FObjectInitializer& ObjectInitializer);

	virtual void DrawGameData(FVector2D &DrawPosition, ASolGameState* InGameState) override;

};
