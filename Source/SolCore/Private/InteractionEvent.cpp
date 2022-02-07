// Copyright Kyle Taylor Lange

#include "InteractionEvent.h"
#include "InteractableComponent.h"

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
