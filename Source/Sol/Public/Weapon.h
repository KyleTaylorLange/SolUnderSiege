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
 * Base class of all handheld usable items.
 */
UCLASS(Abstract)
class SOL_API AWeapon : public AInventoryItem
{
	GENERATED_UCLASS_BODY()

protected:

	/** Mesh material instances. Created so we can manipulate colour, cloaking, etc. **/
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* IdleAnim1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* IdleAnim3P;

public:

	virtual void PostInitializeComponents() override;

	/* Weapon slot type. */
	//UPROPERTY(EditDefaultsOnly, Category = Weapon)
	FName WeaponSlotType;

	/** Projectile class to spawn upon firing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TArray<TSubclassOf<AProjectile>> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimSequence* IdleAnimSeq;

	/** Get the pawn's aim, then adjust it (e.g. for random inaccuracy). */
	virtual FRotator GetAdjustedAimRot() const;
	virtual FVector GetAdjustedAimLoc() const;

	/** Gets the point the weapon should theoretically hit when fired. */
	virtual FVector GetAimPoint() const;

	// How good the AI considers this weapon.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float BaseAIRating;

	// How good the AI considers this weapon in this context.
	UFUNCTION(BlueprintCallable, Category = AI)
	virtual float GetAIRating();
};
