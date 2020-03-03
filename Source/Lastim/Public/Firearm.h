// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapon.h"
#include "Bullet.h"
#include "Firearm.generated.h"

USTRUCT()
struct FFirearmData
{
	GENERATED_USTRUCT_BODY()

	/** Time to reload weapon. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float ReloadDuration;

	/** How fast the weapon can be aimed. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float AimSpeed;

	/** defaults */
	FFirearmData()
	{
		ReloadDuration = 0.5f;
		AimSpeed = 0.5f;
	}
};

USTRUCT()
struct FFireMode
{
	GENERATED_USTRUCT_BODY()

	/* Damage done per shot. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float ShotDamage;

	/** Time between two consecutive shots. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float TimeBetweenShots;

	/** Amount of shots per trigger pull. Zero for fully automatic. ANY OTHER NUMBER IS CURRENTLY SEMI-AUTO. **/
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	int32 ShotsPerBurst;

	/* Ammo consumed per shot. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float AmmoPerShot;

	/* Bullet properties. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	struct FBulletProperties BulletProps;

	/* Projectile to use. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Firearm)
	TSubclassOf<class AProjectile> ProjectileClass;

	/** defaults */
	FFireMode()
	{
		ShotDamage = 25;
		TimeBetweenShots = 0.25f;
		ShotsPerBurst = 1;
		AmmoPerShot = 1;
		BulletProps = FBulletProperties::FBulletProperties();
		ProjectileClass = ABullet::StaticClass();
	}
};

//UCLASS()
//class LASTIM_API AFirearm : public AWeapon
UCLASS(Abstract, config = Game)
class AFirearm : public AWeapon
{
	GENERATED_BODY()

	/** Override StartFire and StopFire to impliment gun-like functionality. **/
	virtual void StartFire() override;
	virtual void StopFire() override;

	/** Override Reload to, umm, reload? **/
	virtual void StartReload(bool bFromReplication = false) override;

	/** Ends weapon reload. **/
	virtual void FinishReload();

	/** Actually reloads weapon's ammo and removes clips. **/
	virtual void ReloadFirearm();

	/** Cancels a reload in progress, such as when the player holding it is killed. */
	virtual void CancelReloadInProgress();

	/** Actually reloads weapon's ammo and removes clips. **/
	virtual class AAmmo* ChooseBestAmmoItem();

	/** Changes the weapon's fire mode. */
	virtual void StartSwitchFireMode(bool bFromReplication = false) override;

	/** Ends weapon reload. **/
	virtual void FinishSwitchFireMode();

	virtual void ChangeFireMode();

	virtual void DetermineWeaponState() override;
	virtual void SetWeaponState(FName NewState) override;

public:

	AFirearm(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaSeconds) override;

	/** Returns Ammo. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetAmmo() const;

	/** Returns MaxAmmo. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetMaxAmmo() const;

	/** Returns the player's ammo as a percentage of max ammo. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual float GetAmmoPct() const;

	/** Returns the player's ammo as a percentage of max ammo. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetAmmoForFireMode(int32 Num = 0) const;

	/** Returns max shots for this fire mode. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetMaxAmmoForFireMode(int32 Num = 0) const;

	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetCurrentFireMode() const;

	// Returns the weapon's aim speed.
	virtual float GetAimSpeed();

	// Returns AimedOffset.
	FVector GetAimedOffset() const;

	// Action called when item enters the player's inventory.
	virtual void OnEnterInventory(ASolCharacter* NewOwner) override;

	virtual FRotator GetAdjustedAimRot() const override;
	//virtual FVector GetAdjustedAimLoc() const override;

	/** Can the weapon fire? **/
	virtual bool CanFire();

	/** Can the weapon reload? **/
	virtual bool CanReload();

	/** Can the weapon switch its fire mode? */
	virtual bool CanSwitchFireMode();

	/* Drop weapon. */
	virtual void OnLeaveInventory();

	// List of fire settings which control how the weapon fires (semi-auto, burst, full-auto, etc).
	UPROPERTY(EditDefaultsOnly, Category = Config)
	TArray<FFireMode> FireMode;

	/* Does the player want to continue a shovel reload? */
	bool bContinueReloading;

	//////////////////////////////////////////////////////////////////////////
	// AI

	/* Gets a default object of the projectile, but with modified characteristics (e.g. custom bullet velocity). */
	virtual AProjectile* GetModifiedProjectile(int32 FireModeNum);

	virtual bool IsFullAuto();

	virtual bool ShouldReload();

protected:

	/** Configuration for this firearm.
	    This is a holdover from ShooterGame and should be eliminated. */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FFirearmData FirearmConfig;

	/** The current fire mode. */
	int32 CurrentFireMode;

	/** The maximum amount of fire modes the player can cycle through.
	    Note that there could be more fire modes than these for use as special cases, such as rifle grenades. */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	int32 MaxFireModes;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	class USoundBase* OutOfAmmoSound;

	// Ammo per second that can transfer to this weapon from the user.
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float RechargeRate;

	// Amount of ammo items to spawn at start.
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	int32 StartingAmmoItems;


public:
	/* Default ammo type to use for this weapon. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	TSubclassOf<class AAmmo> DefaultAmmoClass;

	/* Current ammo/magazine loaded in this weapon. */
	UPROPERTY(Transient, Replicated)
	class AAmmo* CurrentAmmoItem;

	/* Ammo/magazine we want to use on reload. */
	UPROPERTY(Transient, Replicated)
	class AAmmo* PendingAmmoItem;

public:
	/* List of Ammo items for this weapon; temporary until ammo is moved to the Character's inventory. */
	UPROPERTY()
	TArray<class AAmmo*> AmmoInventory;


	// TEMP Variables for new ammo system testing.
	// TEST: Use internal ammo supplies instead of reloading.
	bool bUseInternalAmmo;

	// Ammo stored internally
	int32 Ammo;
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	int32 MaxAmmo;
	int32 ReserveAmmo;
	int32 ReserveAmmoScalar;
	/** Returns max shots for this fire mode. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetReserveAmmoForFireMode(int32 Num = 0) const;

	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float RechargeTime;
	/** Timer handle for time between shots. */
	FTimerHandle FTimerHandle_RechargeIntAmmo;

protected:
	// The spread (in cm) of shots hitting a target at SpreadRange. This should be 5 cm for most non-shotgun weapons. Zero means perfect accuracy.
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float SpreadRadius;

	// Range (in metres) where shots hit within SpreadRadius (generally 5 cm). Zero means perfect accuracy.
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float SpreadRange;

	// Amount of projectiles to spawn per shot. Generally just one for anything but shotgun-like weapons.
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	TArray<int32> ProjectileCount;

	/** Recoil per shot fired. Currently no pitch/yaw customization. **/
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float RecoilPerShot;

	/** The last time a shot was fired. **/
	float LastFireTime;

	/** The last time the weapon recharged a shot. **/
	float RechargePct;

	/** The weapon's desired position when aiming down the sights. */
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	FVector AimedOffset;

	/** Sound to play each time we fire */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	class USoundCue* FireSound;

	// Particle system for muzzle flash.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* FireFX;

	// Particle System Component for FireFX
	UPROPERTY(Transient)
	UParticleSystemComponent* FirePSC;

	// Audio component for FireSound.
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	// Name of bone/socket for the muzzle (for FX)
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	/** Actually fire the weapon (spawn shot, use ammo, etc.) */
	virtual void FireWeapon(float DamageScalar = 1.0f); //PURE_VIRTUAL(AFirearm::FireWeapon, );

	/** Reduce the weapon's ammo supply. */
	virtual int32 UseAmmo(int32 Amount);

	/** Add recoil to the weapon. */
	virtual void AddRecoil();

	/** Start firing. */
	virtual void OnBurstStarted();

	/** Finish firing. */
	virtual void OnBurstFinished();

	/** Timer handle for Finish Reload. */
	FTimerHandle TimerHandle_FinishReload;

	/** Timer handle for Reload Weapon. */
	FTimerHandle TimerHandle_ReloadFirearm;

	/** Timer handle for Reload Weapon. */
	FTimerHandle TimerHandle_FinishCharging;

	/** Timer handle for Reload Weapon. */
	FTimerHandle TimerHandle_FinishSwitchFireMode;

	/** Timer handle for Reload Weapon. */
	FTimerHandle TimerHandle_ChangeFireMode;

	/** Timer handle for time between shots. */
	FTimerHandle FTimerHandle_ShotTimer;

	/** Handle a weapon fire request. */
	void HandleFiring();

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ReloadAnim1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* ReloadAnim3P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* InsertChargerAnim1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* InsertChargerAnim3P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* RemoveChargerAnim1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* RemoveChargerAnim3P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* SwitchFireModeAnim1P;

	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* SwitchFireModeAnim3P;

	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();
	bool ServerHandleFiring_Validate();
	void ServerHandleFiring_Implementation();

	/** Weapon wants to fire (trigger is pulled). **/
	bool bWantsToFire;

	/** Does the weapon want to fire another shot? **/
	bool bRefiring;

	/** A reload is pending. **/
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool bPendingReload;

	/** A fire mode swich is pending. **/
	UPROPERTY(Transient, ReplicatedUsing = OnRep_SwitchFireMode)
	bool bPendingSwitchFireMode;

	/** Spawn projectile on server. **/
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar = 1.f);
	bool ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScalar = 1.f);
	void ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir, float DamageScapar = 1.f);

	/** Add recoil to the pawn holding the weapon. **/
	UFUNCTION(reliable, server, WithValidation)
	void ServerAddRecoilToPawn();
	bool ServerAddRecoilToPawn_Validate();
	void ServerAddRecoilToPawn_Implementation();

	//////////////////////////////////////////////////////////////////////////
	// Replication & effects

	// Burst counter, used for replicating fire events to remote clients.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Reload();

	UFUNCTION()
	void OnRep_SwitchFireMode();

	// Plays the effects for firing the weapon (sound, particles, etc.)
	virtual void SimulateWeaponFire();

	virtual void StopSimulatingWeaponFire();

	virtual void UpdateStatusDisplay();
};
