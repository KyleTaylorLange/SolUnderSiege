// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "UI/SolHUD.h"
#include "HUD_Assassination.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AHUD_Assassination : public ASolHUD
{
	GENERATED_BODY()

	virtual void DrawObjectLabels() override;

	virtual void DrawGameData(FVector2D &DrawPosition, ASolGameState* InGameState) override;
};
