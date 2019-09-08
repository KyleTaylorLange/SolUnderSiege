// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
//#include "LastimCharacter.h"
#include "Pickup.generated.h"

UCLASS()
class LASTIM_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	
	APickup(const FObjectInitializer& ObjectInitializer);

	// Mesh this pickup uses.
	// Note: used to be UMeshComponent, and a SphereComponent was the root.
	class USkeletalMeshComponent* PickupMesh;

	/* Each pickup will have a SphereComp to ensure there is always collision. */
	class UShapeComponent* TempShapeComp;

	// Gets the item this pickup holds. May or may not currently exist in the world, depending upon the subclass.
	virtual class ALastimInventory* GetHeldItem() const;

	// Can this item be picked up?
	virtual bool CanBePickedUp(class ALastimCharacter* TestPawn) const;

	// Give pickup to Pawn. Returns true if obtained.
	virtual void GivePickupTo(class ALastimCharacter* Pawn);

	// Called when pickup is used by a player.
	void PickupOnUse(class ALastimCharacter* Pawn);

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

	// Creates the pickup's mesh.
	virtual void CreatePickupMesh(class ALastimInventory* InItem);
};
