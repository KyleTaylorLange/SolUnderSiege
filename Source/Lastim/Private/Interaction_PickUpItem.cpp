// Copyright Kyle Taylor Lange

#include "Interaction_PickUpItem.h"
#include "InteractableComponent.h"
#include "Pickup.h"
#include "InventoryItem.h"

UInteraction_PickUpItem::UInteraction_PickUpItem(const FObjectInitializer& ObjectInitializer)
{
	//ActionName = FString("Pick up");
}

FString UInteraction_PickUpItem::GetActionName(AActor* Interactor, UObject* Interactee) const
{
	if (UInteractableComponent* Interactable = Cast<UInteractableComponent>(Interactee))
	{
		if (APickup* Pickup = Cast<APickup>(Interactable->GetOwner()))
		{
			return "Pick up " + Pickup->GetHeldItem()->GetDisplayName();
		}
	}
	return Super::GetActionName(Interactor, Interactee);
}
