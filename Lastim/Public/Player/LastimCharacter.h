// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "LastimInventory.h"
#include "LastimCharacter.generated.h"

USTRUCT(BlueprintType)
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY()

	/** Damage amount. */
	UPROPERTY(BlueprintReadWrite, Category = TakeHitInfo)
	int32 ActualDamage;

	/** Type of damage we were hit with. */
	UPROPERTY(BlueprintReadWrite, Category = TakeHitInfo)
	TSubclassOf<UDamageType> DamageTypeClass;

	/** Who hit us. **/
	UPROPERTY()
	TWeakObjectPtr<class ALastimCharacter> PawnInstigator;

	/** Who actually caused the damage. **/
	UPROPERTY()
	TWeakObjectPtr<class AActor> DamageCauser;

	/** Specifies which DamageEvent below describes the damage received. **/
	UPROPERTY()
	int32 DamageEventClassID;

	UPROPERTY()
	bool bKilled;

private:

	/** A rolling counter used to ensure the struct is dirty and will replicate. **/
	UPROPERTY()
	uint8 EnsureReplicationByte;

	/** Describes general damage. **/
	UPROPERTY()
	FDamageEvent GeneralDamageEvent;

	/** Describes point damage, if that is what was received. **/
	UPROPERTY()
	FPointDamageEvent PointDamageEvent;

	/** Describes radial damage, if that is what was received. **/
	UPROPERTY()
	FRadialDamageEvent RadialDamageEvent;

	/** the location of the hit (relative to Pawn center) */
	//UPROPERTY(BlueprintReadWrite, Category = TakeHitInfo)
	//FVector_NetQuantize RelHitLocation;
	/** how much momentum was imparted */
	//UPROPERTY(BlueprintReadWrite, Category = TakeHitInfo)
	//FVector_NetQuantize Momentum;
	/** shot direction pitch, manually compressed */
	//UPROPERTY(BlueprintReadWrite, Category = TakeHitInfo)
	//uint8 ShotDirPitch;
	/** shot direction yaw, manually compressed */
	//UPROPERTY(BlueprintReadWrite, Category = TakeHitInfo)
	//uint8 ShotDirYaw;

public:
	FTakeHitInfo()
		: ActualDamage(0)
		, DamageTypeClass(NULL)
		, PawnInstigator(NULL)
		, DamageCauser(NULL)
		, DamageEventClassID(0)
		, bKilled(false)
		, EnsureReplicationByte(0)
	{}

	FDamageEvent& GetDamageEvent()
	{
		switch (DamageEventClassID)
		{
		case FPointDamageEvent::ClassID:
			if (PointDamageEvent.DamageTypeClass == NULL)
			{
				PointDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return PointDamageEvent;

		case FRadialDamageEvent::ClassID:
			if (RadialDamageEvent.DamageTypeClass == NULL)
			{
				RadialDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return RadialDamageEvent;

		default:
			if (GeneralDamageEvent.DamageTypeClass == NULL)
			{
				GeneralDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
			}
			return GeneralDamageEvent;
		}
	}

	void SetDamageEvent(const FDamageEvent& DamageEvent)
	{
		DamageEventClassID = DamageEvent.GetTypeID();
		switch (DamageEventClassID)
		{
		case FPointDamageEvent::ClassID:
			PointDamageEvent = *((FPointDamageEvent const*)(&DamageEvent));
			break;
		case FRadialDamageEvent::ClassID:
			RadialDamageEvent = *((FRadialDamageEvent const*)(&DamageEvent));
			break;
		default:
			GeneralDamageEvent = DamageEvent;
		}

		DamageTypeClass = DamageEvent.DamageTypeClass;
	}

	void EnsureReplication()
	{
		EnsureReplicationByte++;
	}
};

UCLASS(config=Game)
class ALastimCharacter : public ACharacter
{
	GENERATED_BODY()

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	/** Updates the character. This currently includes: weapon location. */
	virtual void Tick(float DeltaSeconds) override;

	/** [server] perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	/** [client] perform PlayerState related setup */
	virtual void OnRep_PlayerState() override;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	// Name of bone/socket for the held weapon.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName WeaponAttachPoint;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Sets bIsAiming, and thus if the character is aiming. **/
	virtual void SetAiming(bool bNewAiming);

	/** Sets bIsSprinting, and thus if the character is sprinting. **/
	virtual void SetSprinting(bool bNewSprinting);

	// Equip a specific weapon.
	void EquipWeapon(class ALastimWeapon* Weapon);

	// Drop a specific inventory item.
	void DropInventory(class ALastimInventory* Inv);

	// Run through possible USE actions, then execute the best one.
	bool HandleUse();

public:
	ALastimCharacter(const FObjectInitializer& ObjectInitializer);

	// Returns either the first or third person mesh.
	USkeletalMeshComponent* GetSpecificPawnMesh(bool WantFirstPerson) const;

	// True is player is in first person view.
	UFUNCTION(BlueprintCallable, Category = Rendering)
	bool IsFirstPerson() const;

	// Gets the correct mesh (first person or third person).
	USkeletalMeshComponent* GetPawnMesh() const;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Starts firing weapon. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StartFire();

	/** Stops firing weapon. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StopFire();

	/** Initiates reloading. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void OnReload();

	/** Selects a fire mode. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void OnSwitchFireMode();

	/** Starts aiming weapon. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StartAim();

	/** Stops aiming weapon. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StopAim();

	// Player has pressed the USE key; figure out what action to do (if any).
	UFUNCTION(BlueprintCallable, Category = Input)
	bool OnStartUse();

	// Player has depressed the USE key; figure out what action to do (if any).
	UFUNCTION(BlueprintCallable, Category = Input)
	void OnStopUse();

	/** Starts sprinting. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StartSprint();

	/** Stops sprinting. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StopSprint();

	/** Returns bIsAiming. **/
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual bool IsAiming() const;

	/** Returns bIsSprinting. **/
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual bool IsSprinting() const;

	/** Returns length of the player's inventory. **/
	int32 GetInventoryCount() const;

	virtual void DestroyInventory();

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	class UAnimMontage* FireAnimation;

	// Get item we can use at the moment.
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	AActor* GetUsableObject();
	
	/** Get player's current health. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetHealth() const;

	/** Get player's maximum health. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetMaxHealth() const;

	/** Get maximum health that will regenerate. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetShield() const;

	/** Get maximum health that will regenerate. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetMaxShield() const;

	/** Get player's current energy reserve. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetEnergy() const;

	/** Get player's maximum energy. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetMaxEnergy() const;

	/** Use energy from the energy reserves. Returns actual amount used. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float UseEnergy(float Amount);

	/** Add energy to the player's reserves. Returns any remaining energy. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float AddEnergy(float Amount);

	/** Player's armour. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Armour;

	/** Player's stamina, which determines how much they can run (for now). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Stamina;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	ALastimWeapon* GetEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	ALastimWeapon* GetPendingWeapon();

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	ALastimWeapon* GetSpecificWeapon(int32 WeapNum);

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual void EquipSpecificWeapon(int32 WeapNum);

	/** Switches to a weapon based on its place in the weapon list relative to the current weapon. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual void EquipWeaponByDeltaIndex(int32 DeltaIndex);

	UFUNCTION(BlueprintCallable, Category = Effects)
	FName GetWeaponAttachPoint() const;

	/* Gets the head's location. For bot AI. */
	UFUNCTION(BlueprintCallable, Category = AI)
	FVector GetHeadLocation() const;

	/* Gets the torso's location. For bot AI. */
	UFUNCTION(BlueprintCallable, Category = AI)
	FVector GetTorsoLocation() const;

	virtual void TurnOff() override;

	virtual void SetWeaponFiringAllowed(bool bInWeaponFiringAllowed);

	/** Adds the player's starting inventory. **/
	virtual void SpawnInitialInventory(TArray<TSubclassOf<ALastimInventory>> InventoryFromGameMode, bool bUsePawnDefaultInventory = false);

	// Creates and returns new item of class NewItemClass and adds it to the inventory.
	ALastimInventory* CreateNewInventoryItem(TSubclassOf<class ALastimInventory> NewItemClass);

	// Adds item to the inventory.
	void AddToInventory(ALastimInventory* NewItem);

	// Adds item to the inventory.
	void RemoveFromInventory(ALastimInventory* ItemToRemove);

	// Adds item to the inventory.
	void RemakeWeaponList();

	/* Called by the player's previous weapon once it has finished being unequipped. */
	void OnWeaponUnequipFinish(class ALastimWeapon* OldWeapon);

	// Checks if the player can grab the pickup.
	// TODO: Maybe split to see if player:
	//    1. Can add item to inventory (current design), and:
	//    2. Can physically grab the item (too far, has no arms, etc.) 
	bool CanPickupItem(ALastimInventory* Item) const;

	// Checks if the player can pickup an inventory item by dropping another one.
	// Returns the item (if any) that can be swapped.
	ALastimInventory* CanSwapForItem(ALastimInventory* Item) const;

	virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime) override;

	/** Get the final offsets of the player's weapon. */
	virtual FRotator GetWeaponRotationOffset() const;
	virtual FVector GetWeaponLocationOffset() const;

	/** Get the player's current weapon aim. */
	virtual FRotator GetWeaponAimRot() const;
	virtual FVector GetWeaponAimLoc() const;

	virtual void AddRecoil(FVector InRecoil);

	/** Reduce recoil after every tick. */
	void ProcessRecoil(float DeltaSeconds);

	/** Following almost ripped verbatim from ShooterGame. **/

	/** Pawn is in dying state. **/
	bool bIsDying;

	/** check if pawn is still alive */
	bool IsAlive() const;

	/** Returns true if the pawn can die in the current state. **/
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	/**
	* Kills pawn.  Server/authority only.
	* @param KillingDamage - Damage amount of the killing blow
	* @param DamageEvent - Damage event of the killing blow
	* @param Killer - Who killed this pawn
	* @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	* @returns true if allowed
	**/
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	/** notification when killed, for both the server and client. **/
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** Play effects when hit. **/
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	void SetRagdollPhysics();

	virtual void FellOutOfWorld(const UDamageType& DmgType) override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual void TakeFallingDamage(const FHitResult& Hit, float FallingVelocity);

	//////////////////////////////////////////////////////////////////////////
	// Animations
	// Note: three below functions ripped straight from ShooterGame verbatim for the moment.

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

protected:

	/** Player's health. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float Health;

	/** Player's maximum health, generally 100. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float MaxHealth;

	/** Player's health. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float Shield;

	/** Player's maximum health, generally 100. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float MaxShield;

	/** Player's energy reserves. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float Energy;

	/** Highest amount of energy where the player receives 100% of energy pickups. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float FullEnergy;

	/** Player's maximum energy reserve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float MaxEnergy;

	/** Currently equipped weapon. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_EquippedWeapon)
	class ALastimWeapon* EquippedWeapon;

	/** Weapon we want to equip. */
	class ALastimWeapon* PendingWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon(class ALastimWeapon* LastWeapon);

public:
	/** List of inventory items carried by this character. */
	//UPROPERTY(Transient, Replicated)
	TArray<class ALastimInventory*> ItemInventory;

protected:
	/** Roster of weaponry. */
	UPROPERTY(Transient, Replicated)
	TArray<class ALastimWeapon*> WeaponInventory;

	/* The player's sidearm.
	   Only handgun-sized weapons fit in this slot. */
	UPROPERTY(Transient, Replicated)
	ALastimWeapon* SidearmWeapon;

	/* The player's primary weapon. */
	UPROPERTY(Transient, Replicated)
	ALastimWeapon* PrimaryWeapon;

	/* The player's secondary weapon. */
	UPROPERTY(Transient, Replicated)
	ALastimWeapon* SecondaryWeapon;

	/* Other weapons the player can equip, such as grenades. */
	UPROPERTY(Transient, Replicated)
	TArray<class ALastimWeapon*> Equipment;

	/* Max amount of equipment slots. */
	int32 MaxEquipment;

	void SetEquippedWeapon(class ALastimWeapon* NewWeapon, class ALastimWeapon* LastWeapon = NULL);

	/* Sets which weapon the player desires to equip. */
	void SetPendingWeapon(class ALastimWeapon* NewWeapon);

	/** Handler for a touch input beginning. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Mesh material instances. Created so we can manipulate colour, cloaking, etc. **/
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** Mesh material instances. Created so we can manipulate colour, cloaking, etc. **/
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> Mesh1PMIDs;

	/** Switches to a specific weapon. TODO: choose specific weapon. */
	void OnSwitchWeapon(); //(int8 WeapNum)

	/** Switches to the previous weapon in the inventory. */
	void OnPrevWeapon();

	/** Switches to the next weapon in the inventory. */
	void OnNextWeapon();

	/** Switches to the sidearm. */
	void OnEquipSidearm();

	/** Switches to the primary weapon. */
	void OnEquipPrimary();

	/** Switches to the secondary weapon. */
	void OnEquipSecondary();

	/** Starts aiming weapon. */
	void OnStartAim();

	/** Stops aiming weapon. */
	void OnStopAim();

	/** Starts sprinting. */
	void OnStartSprint();

	/** Stops sprinting. */
	void OnStopSprint();

	/** Throws current weapon. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void OnDropWeapon();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** The default weapon classes to spawn. **/
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class ALastimWeapon>> DefaultWeaponClasses;

	/** Recolour a specific Material Instance. */
	void UpdateMaterialColors(UMaterialInstanceDynamic* UseMID);

	/** Sound played when player respawns. */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	class USoundCue* RespawnSound;

	/** Sound played when item is picked up. */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	class USoundCue* PickupItemSound;

	/** Sound played when using an object is unsuccessful. */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	class USoundCue* UseDenialSound;

	/** Sound played when dying. */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	class USoundCue* DyingSound;

	// Modifies damage and does other effects when hitting a specific body part.
	virtual float LocalizeDamage(float Damage, FName HitBone); // All we need for now.

	/** =============== **/
	/** HIT REPLICATION **/
	/** =============== **/

	/** Replicate where this pawn was last hit and damaged. Originally from ShooterGame. **/
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Sets up the replication for taking a hit. Originally from ShooterGame. **/
	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	/** Play hit or death on client. Originally from ShooterGame. **/
	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	/** ================ **/
	/** SERVER FUNCTIONS **/
	/** ================ **/
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetAiming(bool bNewAiming);
	bool ServerSetAiming_Validate(bool bNewAiming);
	void ServerSetAiming_Implementation(bool bNewAiming);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetSprinting(bool bNewSprinting);
	bool ServerSetSprinting_Validate(bool bNewSprinting);
	void ServerSetSprinting_Implementation(bool bNewSprinting);

	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon(class ALastimWeapon* Weapon);
	bool ServerEquipWeapon_Validate(class ALastimWeapon* Weapon);
	void ServerEquipWeapon_Implementation(class ALastimWeapon* Weapon);

	UFUNCTION(reliable, server, WithValidation)
	void ServerDropInventory(class ALastimInventory* Inv);
	bool ServerDropInventory_Validate(class ALastimInventory* Inv);
	void ServerDropInventory_Implementation(class ALastimInventory* Inv);

	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleUse();
	bool ServerHandleUse_Validate();
	void ServerHandleUse_Implementation();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	// Take damage and handle death.
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** Updates Pawn's rotation to the given rotation. Overridden to add weapon lag offset. */
	//virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime = 0.f);

	/** Recolour all the player's Material Instances. */
	void UpdateAllMaterials();

	virtual FLinearColor GetPrimaryColor();

	virtual FLinearColor GetSecondaryColor();

	FLinearColor PrimaryColorOverride;

	FLinearColor SecondaryColorOverride;
	
	// Update the player camera's FOV.
	void UpdateCameraFOV();

	/** Updates the weapon's location and rotation relative to camera. **/
	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

private:

	/** Is the character trying to aim? **/
	UPROPERTY(Transient, Replicated)
	bool bIsAiming;

	/* Can player fire weapons? */
	bool bWeaponFiringAllowed;

	/** The current position of the pawn's weapon, from fully aimed (1) to fully unaimed (0). **/
	//UPROPERTY(Transient, Replicated)
	float CurrentAimPct;

	/** Is the character sprinting? **/
	UPROPERTY(Transient, Replicated)
	bool bIsSprinting;

	FRotator WeaponRotationOffset;

	FVector WeaponLocationOffset;

	//////////////////////////////////////////////////////////////////////////
	// RECOIL SHIT!
	//  This section will be in constant flux as we test things out.

	/* Amount of recoil left to apply. */
	FVector CurrentRecoilVelocity;

	/* Amount of recoil when we started the curve.
		Needed for the curve to work correctly. */
	FVector LastRecoilVelocity;

	/* Percentage of recoil still remaining to apply. */
	float RecoilCurveTime;

	/* How long it takes to apply all recoil. */
	float RecoilTimeScalar;

	/** The weapon's offset relative to the character's view direction. */
	FRotator HeldWeaponOffset;

	/* Last control rotation when we processed free aim.
		Needed to properly apply free aim. */
	FRotator LastControlRotation;

	/* Maximum radius the character's weapon can be offset by free aim. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxFreeAimRadius;

	/* Maximum radius the character's weapon can be offset due to recoil. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxRecoilOffsetRadius;

	/**
	* Add offset to the character's weapon.
	*
	* @param RotationToAdd rotation offset we desire to add.
	* @param MaxPitch absolute value of maximum pitch offset.
	* @param MaxYaw absolute value of maximum yaw offset.
	*
	* @return FRotator of pitch/yaw we were unable to add (exceeded maximum)
	*/
	virtual FRotator AddWeaponOffset(FRotator RotationToAdd, float MaxPitch, float MaxYaw);
};

