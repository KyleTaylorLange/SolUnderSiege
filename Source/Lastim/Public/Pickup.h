// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "InteractableComponent.h"
#include "Pickup.generated.h"

// TODO: Merge sublcass SpecificPickup into me since GenericPickup has been deleted.
//       No point in separating this from its subclass anymore.
UCLASS()
class LASTIM_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APickup(const FObjectInitializer& ObjectInitializer);

	// Mesh this pickup uses.
	class UMeshComponent* PickupMesh;

	/* Each pickup will have a SphereComp to ensure there is always collision. */
	class UPrimitiveComponent* TempShapeComp;

	// Gets the item this pickup holds. May or may not currently exist in the world, depending upon the subclass.
	virtual class AInventoryItem* GetHeldItem() const;

	virtual void SetHeldItem(class AInventoryItem* InItem);

	// Runs CreatePickupMesh on client.
	UFUNCTION()
	virtual void OnRep_HeldItem();

	// Can this item be picked up?
	virtual bool CanBePickedUp(class ASolCharacter* TestPawn) const;

	// Give pickup to Pawn. Returns true if obtained.
	virtual void GivePickupTo(class ASolCharacter* Pawn);

	UFUNCTION()
	virtual bool CanInteractWith(UInteractableComponent* Component, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction);

	UFUNCTION()
	virtual void OnStartUseBy(UInteractableComponent* Component, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction);

	UFUNCTION()
	virtual void OnStopUseBy(UInteractableComponent* Component, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction);

	// Called when pickup is used by a player.
	void PickupOnUse(class ASolCharacter* Pawn);

	virtual void OnPickedUp();

	// When pickup is damaged by something.
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	// When item is destroyed. Overridden to inform the Pickup Spawner.
	virtual void Destroyed() override;

	// Sets the pickup's initial velocity.
	virtual void InitVelocity(FVector InVelocity);

	// PickupSpawner that created this pickup.
	class APickupSpawner* Spawner;

protected:

	// Item this pickup holds.
	UPROPERTY(BlueprintReadWrite, Replicated, ReplicatedUsing = OnRep_HeldItem)
	class AInventoryItem* HeldItem;

	// Creates the pickup's mesh.
	virtual void CreatePickupMesh(class AInventoryItem* InItem);
};
