// Copyright Kyle Taylor Lange

#include "InteractableComponent.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionEvents.Add(UInteractionEvent::StaticClass());
}

bool UInteractableComponent::CanInteractWith_Implementation(AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction)
{
	// If there is a delegate bound, call it.
	if (CanInteractWithDelegate.IsBound())
	{
		return CanInteractWithDelegate.Execute(this, Interactor, Interaction);
	}
	// Otherwise, assume that the InteractionEvent has already validated this action.
	return true;
}

TArray<TSubclassOf<UInteractionEvent>> UInteractableComponent::GetInteractions(AActor* User) const
{
	TArray<TSubclassOf<UInteractionEvent>> Interactions;
	for (int i = 0; i < InteractionEvents.Num(); i++)
	{
		Interactions.Add(InteractionEvents[i]);
	}
	return Interactions;
}

void UInteractableComponent::OnStartUseBy_Implementation(AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction)
{
	OnStartUseByDelegate.ExecuteIfBound(this, Interactor, Interaction);
}

void UInteractableComponent::OnStopUseBy_Implementation(AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction)
{
	OnStopUseByDelegate.ExecuteIfBound(this, Interactor, Interaction);
}
