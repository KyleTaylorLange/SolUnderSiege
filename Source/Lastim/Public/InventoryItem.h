// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "DroppedPickup.h"
#include "InventoryItem.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class LASTIM_API AInventoryItem : public AActor
{
	GENERATED_UCLASS_BODY()

public:

	virtual void PostInitializeComponents() override;

	UFUNCTION(BlueprintCallable)
	virtual FString GetDisplayName() const;

	UFUNCTION(BlueprintCallable)
	float GetMassInKG() const;

	/** Returns the mass of this item in someone's inventory. */
	UFUNCTION(BlueprintCallable)
	float GetMassInInventory() const;

	/** Can this item be equipped? I.e. held in the player's hands? */
	UFUNCTION(BlueprintCallable)
	bool CanBeEquipped() const;
	
	// Action called when item enters the player's inventory.
	virtual void OnEnterInventory(class ASolCharacter* NewOwner);

	// Action called when item leaves the player's inventory.
	virtual void OnLeaveInventory();

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip();

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is being unequipped by owner pawn */
	virtual void OnUnequip();

	/** weapon is holstered by owner pawn */
	virtual void OnUnequipFinished();

	/** Get the current weapon state. */
	FName GetWeaponState() const;

	/** Set the current weapon state to a new one. */
	virtual void SetWeaponState(FName NewState);

	/** Determines the state the weapon should be in, then initiates that state. **/
	virtual void DetermineWeaponState();

	/** Actions upon pressing and releasing the trigger, respectively. **/
	virtual void StartFire();
	virtual void StopFire();

	/** Actions upon pressing and releasing the reload key, respectively. **/
	virtual void StartReload(bool bFromReplication = false);
	virtual void StopReload(); //bFromReplication unnecessary here?

	/** Actions upon pressing and releasing the switch fire mode key, respectively. **/
	virtual void StartSwitchFireMode(bool bFromReplication = false);
	virtual void StopSwitchFireMode(); //bFromReplication unnecessary here?

	// Sets the inventory item's owner.
	void SetOwningPawn(class ASolCharacter* NewOwner);

	// Retrieves the inventory item's owner.
	//UFUNCTION(BlueprintCallable)
	ASolCharacter* GetOwningPawn();

	// Returns Mesh1P.
	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual USkeletalMeshComponent* GetMesh1P() const;

	// Returns Mesh3P.
	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual USkeletalMeshComponent* GetMesh3P() const;

	// Gets the mesh a pickup of this inventory item should use.
	UFUNCTION(BlueprintCallable)
	virtual UMeshComponent* GetPickupMesh();

	// Gets the appropriate weapon mesh.
	UFUNCTION(BlueprintCallable, Category = Mesh)
	class USkeletalMeshComponent* GetMesh() const;

	/** Offset of player's arms when holding this item. */
	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual FVector GetMeshOffset() const;

	/** Attach mesh to pawn when equipped. **/
	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual void AttachMeshToPawn(FName AttachPoint);

	/** Detatch mesh from pawn when unequipped. **/
	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual void DetachMeshFromPawn();

	/** play weapon animations */
	UFUNCTION(BlueprintCallable, Category = Mesh)
	float PlayWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P);

	/** stop playing weapon animations */
	UFUNCTION(BlueprintCallable, Category = Mesh)
	void StopWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P);

	// Class this inventory item should use for dropped pickups.
	//UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ADroppedPickup> DroppedPickupClass;

	/** Modify the damage given to the player. Default implementation does nothing. */
	//UFUNCTION(BlueprintImplementableEvent)
	virtual void ModifyDamageTaken(float& Damage, TSubclassOf<UDamageType> DamageType);

	// When item is destroyed. Overridden to inform the Pickup Spawner.
	virtual void Destroyed() override;

protected:

	// Human readable name for this item.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	FText DisplayName;

	/** The mass of this inventory item. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Inventory)
	float MassInKG;

	/** How much to multiply this item's MassInKG to determine its weight in the player's inventory. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Inventory)
	float InventoryMassScalar;

	/** Can this item be equipped? **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	bool bCanBeEquipped;

	/** Is this item currently equipped? **/
	bool bIsEquipped;

	/** Is the item being equipped? */
	bool bPendingEquip;

	/** Is the item being unequipped? */
	bool bPendingUnequip;

	/** The name of the weapon's current state.
		This is an FName to allow expansion in subclasses.
		By default we only have Idle and Equipping. */
	FName CurrentState;

	/** Handle for efficient management of OnEquipFinished timer. */
	FTimerHandle TimerHandle_OnEquipFinished;

	/** Handle for efficient management of OnUnequipFinished timer. */
	FTimerHandle TimerHandle_OnUnequipFinished;

	/** Last time weapon started to be equipped. */
	float EquipStartedTime;

	/** Last time weapon started to be unequipped. */
	float UnequipStartedTime;

	/** Time it takes to equip the weapon. */
	float EquipDuration;

	/** Time it takes to equip the weapon. */
	float UnequipDuration;

	// The inventory item's owner.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	class ASolCharacter* MyPawn;

	/** Weapon mesh: 1st person view (arms; seen only by self). **/
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Weapon mesh: 3st person view (arms; seen only by others). **/
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh3P;

	/** Mesh used for pickups. **/
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	class UMeshComponent* PickupMesh;

	/** Offset of player's arm mesh when holding this item. */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	FVector MeshOffset;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* UnequipAnim;

	UFUNCTION()
	void OnRep_MyPawn();

	/* Server Functions */

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartReload();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopReload();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartSwitchFireMode();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopSwitchFireMode();
};
