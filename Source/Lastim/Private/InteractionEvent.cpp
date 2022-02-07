// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "InteractableComponent.h"
#include "Pickup.h"
#include "InventoryItem.h"
#include "InteractionEvent.h"

UInteractionEvent::UInteractionEvent(const FObjectInitializer& ObjectInitializer)
{
	//ActionName = FString("Generic Use Action");
}

FString UInteractionEvent::GetActionName(AActor* Interactor, UObject* Interactee) const
{
	return "";
}

bool UInteractionEvent::CanInteract(AActor* Interactor, AActor* Interactee) const
{
	return false;
}

bool UInteractionEvent::CanInteract(AActor* Interactor, UInteractableComponent* Interactee) const
{
	return Interactee && Interactee->CanInteractWith(Interactor, this->GetClass());
}

void UInteractionEvent::OnStartUse(AActor* Interactor, AActor* Interactee) const
{
	if (UInteractableComponent* Interactable = Cast<UInteractableComponent>(Interactee))
	{
		Interactable->OnStartUseBy(Interactor, this->GetClass());
	}
}

void UInteractionEvent::OnStopUse(AActor* Interactor, AActor* Interactee) const
{
	if (UInteractableComponent* Interactable = Cast<UInteractableComponent>(Interactee))
	{
		Interactable->OnStopUseBy(Interactor, this->GetClass());
	}
}

UInteractionEvent_PickUpItem::UInteractionEvent_PickUpItem(const FObjectInitializer& ObjectInitializer)
{
	//ActionName = FString("Pick up");
}

UInteractionEvent_SwapForItem::UInteractionEvent_SwapForItem(const FObjectInitializer& ObjectInitializer)
{
	//ActionName = FString("Swap for");
}

UInteractionEvent_CapturePoint::UInteractionEvent_CapturePoint(const FObjectInitializer& ObjectInitializer)
{
	//ActionName = FString("Capture point");
}


FString UInteractionEvent_PickUpItem::GetActionName(AActor* Interactor, UObject* Interactee) const
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


FString UInteractionEvent_SwapForItem::GetActionName(AActor* Interactor, UObject* Interactee) const
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


FString UInteractionEvent_CapturePoint::GetActionName(AActor* Interactor, UObject* Interactee) const
{
	return Super::GetActionName(Interactor, Interactee);
}