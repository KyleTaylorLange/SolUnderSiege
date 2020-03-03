// Copyright Kyle Taylor Lange

#pragma once

#include "InventoryItem.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API AAmmo : public AInventoryItem
{
	GENERATED_UCLASS_BODY()
	
public:

	/** Weapon mesh: 3st person view (arms; seen only by others). **/
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh3P;

	virtual FString GetDisplayName() const override;

	/* Returns Ammo Count. */
	virtual int32 GetAmmoCount() const;

	/* Returns Max Ammo. */
	virtual int32 GetMaxAmmo() const;

	// Sets max ammo to a value.
	virtual void SetMaxAmmo(int32 NewMaxAmmo);

	/* Consume ammo in this item. Returns amount used. */
	virtual int32 UseAmmo(int32 InAmmo);

	// Adds ammo to this item. Returns amount successfully added.
	virtual int32 AddAmmo(int32 InAmmo);

	// Will this Ammo recharge over time?
	virtual bool RechargesAmmo() const;

	/* Recharges ammo, and continues recharging if necessary. */
	virtual void OnAmmoRecharge();

	/* List of tags for this ammo type. Used by weapons to determine if they're compatible. */
	TArray<FName> AmmoTags;

	// Amount of ammo to charge per second if under listed amount. 
	//   An X value of 10 and Y value of 100 will charge 10 ammo per second until there is 100 ammo.
	// No ammo brackets means no recharging. A Y value of -1 means rate applies forever.
	TArray<FVector2D> RechargeRateBracket;

	// Amount of ammo to charge per cycle.
	UPROPERTY()
	int32 RechargeAmount;

	float CalculateNextRechargeTime() const;

	/* Called when unloaded from weapon. */
	virtual void OnLoadIntoWeapon();

	/* Called when unloaded from weapon. */
	virtual void OnUnloadFromWeapon();

	virtual void OnEnterInventory(class ASolCharacter* NewOwner) override;

	// Creates a mesh for this item's pickup.
	virtual UMeshComponent* GetPickupMesh() override;

protected:
	
	/* Ammo held in this item. */
	UPROPERTY(Transient, Replicated)
	int32 AmmoCount;

	/* Maximum ammo this item can hold. */
	UPROPERTY()
	int32 MaxAmmo;

	/* Ammo that is lost on removing from weapon (e.g. fuel leaking). */
	UPROPERTY()
	int32 AmmoLostOnUnload;

	/* Timer handle to recharge ammo. */
	FTimerHandle TimerHandle_AmmoRecharge;
};
