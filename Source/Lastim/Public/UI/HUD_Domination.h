// Copyright Kyle Taylor Lange

#pragma once

#include "UI/SolHUD.h"
#include "HUD_Domination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AHUD_Domination : public ASolHUD
{
	GENERATED_BODY()
	
	/* Draws game-specific objectives. Blank in the default class. */
	virtual void DrawObjectiveInfo(ASolGameState* InGameState) override;
	
	
};
