// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "InventoryItem.h"
#include "Projectile.h"
#include "Weapon.generated.h"

namespace WeaponSlotType
{
	const FName Sidearm = FName(TEXT("Sidearm"));
	const FName Main = FName(TEXT("Main"));
	const FName Equipment = FName(TEXT("Equipment"));
}

/**
 * Weapon
 * Base class of all handheld usable idems.
 */
UCLASS(Abstract)
class LASTIM_API AWeapon : public AInventoryItem
{
	GENERATED_BODY()

	/** Weapon mesh: 1st person view (arms; seen only by self). **/
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Weapon mesh: 3st person view (arms; seen only by others). **/
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh3P;

protected:
	
	/** Attach and detach mesh to pawn, respectively. **/
	void AttachMeshToPawn();
	void DetachMeshFromPawn();

	/** Mesh material instances. Created so we can manipulate colour, cloaking, etc. **/
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** Is the weapon currently equipped? **/
	bool bIsEquipped;

	/** Is the weapon being equipped? */
	bool bPendingEquip;

	/** Is the weapon being unequipped? */
	bool bPendingUnequip;

	/** The name of the weapon's current state.
	    This is an FName to allow expansion in subclasses.
		By default we only have Idle and Equipping. **/
	FName CurrentState;

	// Offset of player's arms when holding weapon.
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	FVector WeaponOffset;

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

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* IdleAnim1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* IdleAnim3P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* UnequipAnim;

	/** ================ **/
	/** SERVER FUNCTIONS **/
	/** ================ **/

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartFire();
	bool ServerStartFire_Validate();
	void ServerStartFire_Implementation();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopFire();
	bool ServerStopFire_Validate();
	void ServerStopFire_Implementation();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartReload();
	bool ServerStartReload_Validate();
	void ServerStartReload_Implementation();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopReload();
	bool ServerStopReload_Validate();
	void ServerStopReload_Implementation();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartSwitchFireMode();
	bool ServerStartSwitchFireMode_Validate();
	void ServerStartSwitchFireMode_Implementation();

public:

	AWeapon(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	/** Actions upon pressing and releasing the trigger, respectively. **/
	virtual void StartFire();
	virtual void StopFire();

	/** Actions upon pressing and releasing the reload key, respectively. **/
	virtual void StartReload(bool bFromReplication = false);
	virtual void StopReload(); //bFromReplication unnecessary here?

	/** Actions upon pressing and releasing the switch fire mode key, respectively. **/
	virtual void StartSwitchFireMode(bool bFromReplication = false);
	//virtual void StopSwitchFireMode(); //bFromReplication unnecessary here?

	/** Determine the weapon's current state, then initiate that state. **/
	virtual void DetermineWeaponState();
	FName GetWeaponState();
	virtual void SetWeaponState(FName NewState);

	/* Weapon slot type. */
	//UPROPERTY(EditDefaultsOnly, Category = Weapon)
	FName WeaponSlotType;

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip();

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** weapon is being unequipped by owner pawn */
	virtual void OnUnequip();

	/** weapon is holstered by owner pawn */
	virtual void OnUnequipFinished();

	/** Projectile class to spawn upon firing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TArray<TSubclassOf<AProjectile>> ProjectileClass;

	/** play weapon animations */
	float PlayWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P);

	/** stop playing weapon animations */
	void StopWeaponAnimation(UAnimMontage* Anim1P, UAnimMontage* Anim3P);

	/** Get the pawn's aim, then adjust it (e.g. for random inaccuracy). */
	virtual FRotator GetAdjustedAimRot() const;
	virtual FVector GetAdjustedAimLoc() const;

	// Action called when item enters the player's inventory.
	virtual void OnEnterInventory(ASolCharacter* NewOwner) override;
	// Action called when item leaves the player's inventory.
	virtual void OnLeaveInventory() override;

	// How good the AI considers this weapon.
	float BaseAIRating;

	// How good the AI considers this weapon in this context.
	UFUNCTION(BlueprintCallable, Category = AI)
	virtual float GetAIRating();

	// Returns the mesh a pickup of this weapon should use.
	virtual UMeshComponent* GetPickupMesh() override;

	// Returns Mesh1P.
	UFUNCTION(BlueprintCallable, Category = Mesh)
	USkeletalMeshComponent* GetMesh1P() const;
	// Returns Mesh3P.
	UFUNCTION(BlueprintCallable, Category = Mesh)
	USkeletalMeshComponent* GetMesh3P() const;

	// Gets the appropriate weapon mesh.
	UFUNCTION(BlueprintCallable, Category = Mesh)
	USkeletalMeshComponent* GetWeaponMesh() const;

	// Returns AimedOffset.
	UFUNCTION(BlueprintCallable, Category = Mesh)
	FVector GetWeaponOffset() const;
};
