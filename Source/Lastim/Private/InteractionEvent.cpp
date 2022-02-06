// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "InteractableComponent.h"
#include "InteractionEvent.h"

UInteractionEvent::UInteractionEvent(const FObjectInitializer& ObjectInitializer)
{
	ActionName = FString("Generic Use Action");
}

FString UInteractionEvent::GetActionName() const
{
	return ActionName;
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
	ActionName = FString("Pickup");
}

UInteractionEvent_SwapForItem::UInteractionEvent_SwapForItem(const FObjectInitializer& ObjectInitializer)
{
	ActionName = FString("Swap For");
}

UInteractionEvent_CapturePoint::UInteractionEvent_CapturePoint(const FObjectInitializer& ObjectInitializer)
{
	ActionName = FString("Capture Point");
}
