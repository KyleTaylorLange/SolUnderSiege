// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractionEvent.h"
#include "Interaction_SwapForItem.generated.h"

UCLASS()
class LASTIM_API UInteraction_SwapForItem : public UInteractionEvent
{
	GENERATED_UCLASS_BODY()

	FString GetActionName(AActor* Interactor = nullptr, UObject* Interactee = nullptr) const override;
};
