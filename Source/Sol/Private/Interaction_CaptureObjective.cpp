// Copyright Kyle Taylor Lange

#include "Interaction_CaptureObjective.h"
#include "InteractableComponent.h"
#include "Pickup.h"
#include "InventoryItem.h"

UInteraction_CaptureObjective::UInteraction_CaptureObjective(const FObjectInitializer& ObjectInitializer)
{
	//ActionName = FString("Capture point");
}

FString UInteraction_CaptureObjective::GetActionName(AActor* Interactor, UObject* Interactee) const
{
	return Super::GetActionName(Interactor, Interactee);
}