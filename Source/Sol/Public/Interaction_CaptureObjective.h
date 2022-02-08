// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractionEvent.h"
#include "Interaction_CaptureObjective.generated.h"

UCLASS()
class SOL_API UInteraction_CaptureObjective : public UInteractionEvent
{
	GENERATED_UCLASS_BODY()

	FString GetActionName(AActor* Interactor = nullptr, UObject* Interactee = nullptr) const override;
};
