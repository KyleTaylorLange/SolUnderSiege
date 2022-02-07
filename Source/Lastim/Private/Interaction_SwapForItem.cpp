// Copyright Kyle Taylor Lange

#include "Interaction_SwapForItem.h"
#include "Pickup.h"
#include "InventoryItem.h"

UInteraction_SwapForItem::UInteraction_SwapForItem(const FObjectInitializer& ObjectInitializer)
{
	//ActionName = FString("Swap for");
}

FString UInteraction_SwapForItem::GetActionName(AActor* Interactor, UObject* Interactee) const
{
	if (UInteractableComponent* Interactable = Cast<UInteractableComponent>(Interactee))
	{
		if (APickup* Pickup = Cast<APickup>(Interactable->GetOwner()))
		{
			return "Swap for " + Pickup->GetHeldItem()->GetDisplayName();
		}
	}
	return Super::GetActionName(Interactor, Interactee);
}
