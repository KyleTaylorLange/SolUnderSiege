// Copyright Kyle Taylor Lange

#pragma once

#include "UI/LastimHUD.h"
#include "HUD_Domination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AHUD_Domination : public ALastimHUD
{
	GENERATED_BODY()
	
	/* Draws game-specific objectives. Blank in the default class. */
	virtual void DrawObjectiveInfo(ALastimGameState* InGameState) override;
	
	
};
