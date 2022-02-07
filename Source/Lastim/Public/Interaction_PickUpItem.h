// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractionEvent.h"
#include "Interaction_PickUpItem.generated.h"

// TODO: Move these to separate header and source files, or move them to objects that use them.
UCLASS()
class LASTIM_API UInteraction_PickUpItem : public UInteractionEvent
{
	GENERATED_UCLASS_BODY()

		FString GetActionName(AActor* Interactor = nullptr, UObject* Interactee = nullptr) const override;
};

