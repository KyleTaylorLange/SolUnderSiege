// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "InteractionEvent.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

DECLARE_DELEGATE_RetVal_ThreeParams(bool, FCanInteractWithDelegate, UInteractableComponent*, AActor*, TSubclassOf<UInteractionEvent>)

DECLARE_DELEGATE_ThreeParams(FOnStartUseByDelegate, UInteractableComponent*, AActor*, TSubclassOf<UInteractionEvent>)

DECLARE_DELEGATE_ThreeParams(FOnStopUseByDelegate, UInteractableComponent*, AActor*, TSubclassOf<UInteractionEvent>)

/**
 * An InteractableComponent offers certain InteractionEvents to a prospective interactor
 * and may call delegate functions to refine how interaction works.
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOLCORE_API UInteractableComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = Interactable)
	TArray<TSubclassOf<UInteractionEvent>> InteractionEvents;

	// Can the incoming user use this item?
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	bool CanInteractWith(AActor* Interactor, TSubclassOf<class UInteractionEvent> Interaction);

	TArray<TSubclassOf<UInteractionEvent>> GetInteractions(AActor* User) const;

	FCanInteractWithDelegate CanInteractWithDelegate;

	FOnStartUseByDelegate OnStartUseByDelegate;

	FOnStopUseByDelegate OnStopUseByDelegate;

	/** When someone starts using this item (i.e. use is held down). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnStartUseBy(AActor* User, TSubclassOf<UInteractionEvent> Interaction);

	/** When someone stops using this item (i.e. use is released). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
	void OnStopUseBy(AActor* User, TSubclassOf<UInteractionEvent> Interaction);
};
