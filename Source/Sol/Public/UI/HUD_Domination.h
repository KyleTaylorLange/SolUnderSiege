// Copyright Kyle Taylor Lange

#pragma once

#include "UI/SolHUD.h"
#include "HUD_Domination.generated.h"

/**
 * 
 */
UCLASS()
class SOL_API AHUD_Domination : public ASolHUD
{
	GENERATED_BODY()

	virtual void DrawObjectLabels() override;
	
	/* Draws game-specific objectives. Blank in the default class. */
	virtual void DrawGameData(FVector2D &DrawPosition, ASolGameState* InGameState) override;
	
	
};
