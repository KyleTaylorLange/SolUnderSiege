// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Weapon.h"
#include "Bullet.h"
#include "Projectile.h"
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

	/** Slot to use ammo from. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	int32 AmmoSlot;

	/* Bullet properties. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	struct FBulletProperties BulletProps;

	/* Projectile to spawn when firing. If null, the weapon will be hitscan/instant hit. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	TSubclassOf<AProjectile> ProjectileClass;

	/** Number of projectiles to spawn. Generally just one for all but shotgun-like weapons. */
	int32 ProjectileCount;

	/** defaults */
	FFireMode()
	{
		ShotDamage = 25;
		TimeBetweenShots = 0.25f;
		ShotsPerBurst = 1;
		AmmoPerShot = 1.f;
		AmmoSlot = 0;
		BulletProps = FBulletProperties();
		ProjectileClass = ABullet::StaticClass();
		ProjectileCount = 1;
	}
};

//class LASTIM_API AFirearm : public AWeapon
UCLASS(Abstract, config = Game)
class AFirearm : public AWeapon
{
	GENERATED_UCLASS_BODY()

	/** Override StartFire and StopFire to impliment gun-like functionality. **/
	virtual void StartFire() override;
	virtual void StopFire() override;

	/** Override Reload to, umm, reload? **/
	virtual void StartReload(bool bFromReplication = false) override;

	/** Ends weapon reload. **/
	virtual void FinishReload();

	/** Actually reloads weapon's ammo and removes clips. **/
	UFUNCTION()
	virtual void ReloadFirearm(int SlotIndex = 0);

	/** Cancels a reload in progress, such as when the player holding it is killed. */
	virtual void CancelReloadInProgress();

	/** Actually reloads weapon's ammo and removes clips. **/
	virtual class AAmmo* ChooseBestAmmoItem(int Index = 0);

	/** Changes the weapon's fire mode. */
	virtual void StartSwitchFireMode(bool bFromReplication = false) override;

	/** Ends weapon reload. **/
	virtual void FinishSwitchFireMode();

	virtual void ChangeFireMode();

	virtual void DetermineWeaponState() override;
	virtual void SetWeaponState(FName NewState) override;

public:

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaSeconds) override;

	/** Returns Ammo. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetAmmo(int32 SlotNum = 0) const;

	/** Returns MaxAmmo. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual int32 GetMaxAmmo(int32 SlotNum = 0) const;

	/** Returns the player's ammo as a percentage of max ammo. **/
	UFUNCTION(BlueprintCallable, Category = Weapon)
	virtual float GetAmmoPct(int32 SlotNum = 0) const;

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

	// Calculate a new spread offset in a cone. Can be a whole cone or just a subsection of the cone.
	virtual FRotator CalculateSpread() const;

	/** Can the weapon fire? **/
	virtual bool CanFire();

	/** Can the weapon reload? **/
	virtual bool CanReload();

	/** Can the weapon switch its fire mode? */
	virtual bool CanSwitchFireMode();

	/* Drop weapon. */
	virtual void OnLeaveInventory();

	// List of fire settings which control how the weapon fires (semi-auto, burst, full-auto, etc).
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	TArray<FFireMode> FireMode;

	//////////////////////////////////////////////////////////////////////////
	// AI

	/* Gets a default object of the projectile, but with modified characteristics (e.g. custom bullet velocity). */
	virtual AProjectile* GetModifiedProjectile(int32 FireModeNum);

	virtual bool IsFullAuto();

	virtual bool ShouldReload();

	/** Temp value to determine whether to use one of two ammo systems.
      * Simplified ammo follows the "one bullet clips" trope; your ammo is a pool which is topped off by reloading.
      * The regular system has ammo tracked via magazines; reloading ejects the current one for a different one.
      */
	inline static bool UseSimplifiedAmmo() { return false;  }

protected:

	/** Configuration for this firearm.
	    This is a holdover from ShooterGame and should be eliminated. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	FFirearmData FirearmConfig;

	/** The current fire mode. */
	UPROPERTY(Replicated)
	int32 CurrentFireMode;

	/** The maximum amount of fire modes the player can cycle through.
	    Note that there could be more fire modes than these for use as special cases, such as rifle grenades. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	int32 MaxFireModes;

	UPROPERTY(EditDefaultsOnly, Category = Sound)
	class USoundBase* OutOfAmmoSound;


public:
	/* Default ammo type to use for this weapon. */
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	TArray<TSubclassOf<class AAmmo>> DefaultAmmoClass;

	/* Current ammo items loaded into weapon.. */
	UPROPERTY(Transient, Replicated)
	TArray<class AAmmo*> CurrentAmmoItem;

	/* Ammo/magazine we want to use on reload. */
	UPROPERTY(Transient, Replicated)
	TArray<class AAmmo*> PendingAmmoItem;

public:

	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float MaxHeat;
	float Heat;
	float CooldownDelay;

protected:
	// The spread (in cm) of shots hitting a target at SpreadRange. This should be 5 cm for most non-shotgun weapons. Zero means perfect accuracy.
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float SpreadRadius;

	// Range (in metres) where shots hit within SpreadRadius (generally 5 cm). Zero means perfect accuracy.
	UPROPERTY(EditDefaultsOnly, Category = Firearm)
	float SpreadRange;

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
	virtual float UseAmmo(float Amount, int SlotIndex = 0);

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
	FTimerDelegate TimerDel_ReloadFirearm;

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

	/** Spawn projectile on server. **/
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireHitscan(FVector Origin, FVector_NetQuantizeNormal ShootDir, float TraceLength, float DamageScalar = 1.f);

	/** Add recoil to the pawn holding the weapon. **/
	UFUNCTION(reliable, server, WithValidation)
	void ServerAddRecoilToPawn();

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

	UCanvasRenderTarget2D* ScreenRenderTarget;

public:
	UFUNCTION()
	virtual void DrawCanvasStatusDisplayElements(UCanvas* Canvas, int32 Width, int32 Height);

	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UFont* ScreenDisplayFont;
};
