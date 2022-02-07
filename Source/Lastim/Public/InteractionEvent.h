// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InteractionEvent.generated.h"

/**
 * Describes an interaction between an interactor and and interactee.
 * Interactees may be InteractableComponents that handle some of the behaviour
 * on their own, but interactees may be other objects where all behaviour
 * originates from the interactor and the InteractionEvent.
 * 
 * In essence, the handling of any interaction method flows like so:
 * Interactor -> InteractionEvent -> (optional)InteractableComponent
 */
UCLASS()
class LASTIM_API UInteractionEvent : public UObject
{
	GENERATED_UCLASS_BODY()

	virtual FString GetActionName(AActor* Interactor = nullptr, UObject* Interactee = nullptr) const;

	virtual bool CanInteract(AActor* Interactor, AActor* Interactee) const;

	virtual bool CanInteract(AActor* Interactor, class UInteractableComponent* Interactee) const;

	virtual void OnStartUse(AActor* Interactor, AActor* Interactee) const;

	virtual void OnStopUse(AActor* Interactor, AActor* Interactee) const;
};

// TODO: Move these to separate header and source files, or move them to objects that use them.
UCLASS()
class LASTIM_API UInteractionEvent_PickUpItem : public UInteractionEvent
{
	GENERATED_UCLASS_BODY()

		FString GetActionName(AActor* Interactor = nullptr, UObject* Interactee = nullptr) const override;
};

UCLASS()
class LASTIM_API UInteractionEvent_SwapForItem : public UInteractionEvent
{
	GENERATED_UCLASS_BODY()

	FString GetActionName(AActor* Interactor = nullptr, UObject* Interactee = nullptr) const override;
};

UCLASS()
class LASTIM_API UInteractionEvent_CapturePoint : public UInteractionEvent
{
	GENERATED_UCLASS_BODY()

	FString GetActionName(AActor* Interactor = nullptr, UObject* Interactee = nullptr) const override;
};
