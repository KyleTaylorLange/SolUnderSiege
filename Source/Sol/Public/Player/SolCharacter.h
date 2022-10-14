// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
// Also, copy Kyle Taylor Lange

#pragma once
#include "GameFramework/Character.h"
#include "SolCharacter.generated.h"



// This struct was mostly taken from the ShooterGame example project.
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
	TWeakObjectPtr<class ASolCharacter> PawnInstigator;

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
				PointDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : (TSubclassOf<UDamageType>)UDamageType::StaticClass();
			}
			return PointDamageEvent;

		case FRadialDamageEvent::ClassID:
			if (RadialDamageEvent.DamageTypeClass == NULL)
			{
				RadialDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : (TSubclassOf<UDamageType>)UDamageType::StaticClass();
			}
			return RadialDamageEvent;

		default:
			if (GeneralDamageEvent.DamageTypeClass == NULL)
			{
				GeneralDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : (TSubclassOf<UDamageType>)UDamageType::StaticClass();
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

/** The different sections of the body in human-readable form. */
UENUM()
enum EBodySection
{
	Head,
	UpperTorso,
	LowerTorso,
	LeftLeg,
	RightLeg,
	LeftArm,
	RightArm,
	LeftTorso,
	RightTorso,
};

/** A struct representing info about a section of the player's body. */
USTRUCT()
struct FBodySectionInfo
{
	GENERATED_BODY()

	/** Names of the mesh bones belonging to this section. */
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	TArray<FName> Bones;
};

UCLASS(config=Game)
class ASolCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

private:

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	/** Updates the character. This currently includes weapon location. */
	virtual void Tick(float DeltaSeconds) override;

	/** [server] perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	/** [client] perform PlayerState related setup */
	virtual void OnRep_PlayerState() override;

	/** Pawn mesh: 1st person view (e.g. the arms seen in first person). */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Mesh for the character's helmet. Shouldn't require animation, so it can be a static mesh. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* HelmetMesh;

	// Name of bone/socket for the held weapon.
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	FName WeaponAttachPoint;

	// Name of bone/socket for the helmet.
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	FName HelmetAttachPoint;

	// Sockets to attach weapon meshes.
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	FName RightThighAttachPoint;
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	FName LeftThighAttachPoint;
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	FName BackAttachPoint;

	/** Component responsible for handling inventory. */
	UPROPERTY(VisibleDefaultsOnly, Category = Gameplay)
	class UInventoryComponent* InventoryComponent;

	/** The different sections of the player's body. */
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	TArray<FBodySectionInfo> BodySections;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Sets bIsAiming, and thus if the character is aiming. **/
	virtual void SetAiming(bool bNewAiming);

	/** Sets bIsSprinting, and thus if the character is sprinting. **/
	virtual void SetSprinting(bool bNewSprinting);

	// Run through possible USE actions, then executes the best one.
	void HandleUse();

public:

	// Equip a specific weapon.
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void EquipItem(class AInventoryItem* Item);

	// Drop a specific inventory item.
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void DropInventory(class AInventoryItem* Inv);

	// Returns either the first or third person mesh.
	USkeletalMeshComponent* GetSpecificPawnMesh(bool WantFirstPerson) const;

	// True if player is in first person view.
	UFUNCTION(BlueprintCallable, Category = Rendering)
	bool IsFirstPerson() const;

	/** Returns	the pawn's eye location. */
	FVector GetPawnViewLocation() const override;

	// Gets the correct mesh (first person or third person).
	USkeletalMeshComponent* GetPawnMesh() const;

	/**
	 * Gets the InventoryComponent
	 * @return The inventory component.
	 */
	UFUNCTION(BlueprintCallable, Category = Inventory)
	class UInventoryComponent* GetInventoryComponent() const;

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
	void OnStartUse();

	// Player has depressed the USE key; figure out what action to do (if any).
	UFUNCTION(BlueprintCallable, Category = Input)
	void OnStopUse();

	/** Starts sprinting. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StartSprint();

	/** Stops sprinting. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void StopSprint();

	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable, Category = Input)
	void MoveForward(float Val);

	/** Handles strafing movement, left and right */
	UFUNCTION(BlueprintCallable, Category = Input)
	void MoveRight(float Val);

	/** Handles moving up and down. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void MoveUp(float Val);

	/** Returns bIsAiming. **/
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual bool IsAiming() const;

	/* Returns CurrentAimPct, with 1.0 being aimed and 0.0 being totally unaimed. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual float GetAimPct() const;

	/** Returns bIsSprinting. **/
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual bool IsSprinting() const;

	// Get item we can use at the moment.
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	class UInteractableComponent* FindInteractable(TSubclassOf<UInteractionEvent>& Interaction);
	
	/** Get player's current health. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetHealth() const;

	/** Get player's maximum health that will regenerate. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetCappedHealth() const;

	/** Get player's maximum possible health. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetFullHealth() const;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetMaxHealth() const;

	/** Get player's current leg health. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetLegHealth() const;

	/** Set's the players health to a new value. 
	  * This is not to be used for damage or healing.
	  * @param Health The new health value. 
	  */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual void SetHealth(float Health);

	/** Set's the players max health to a new value.
	  * @param FullHealth The new full health value.
	  */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual void SetFullHealth(float FullHealth);

	/** Set's the players full health to a new value.
	  * This is not to be used for damage or healing.
	  * @param MaxHealth The new max health value.
	  */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual void SetMaxHealth(float MaxHealth);

	/** Get player's full leg health. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	float GetFullLegHealth() const;

	/** Player's stamina, which determines how much they can run (for now). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float Stamina;

	/** The maximum amount of stamina the character can have. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxStamina;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	class AInventoryItem* GetEquippedItem() const;

	UFUNCTION(BlueprintCallable, Category = Gameplay)
	class AInventoryItem* GetPendingItem();

	/** Switches to a weapon based on its place in the weapon list relative to the current weapon. */
	UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual void EquipWeaponByDeltaIndex(int32 DeltaIndex);

	UFUNCTION(BlueprintCallable, Category = Effects)
	FName GetWeaponAttachPoint() const;

	UFUNCTION(BlueprintCallable, Category = Effects)
	virtual FName DetermineItemAttachPoint(class AInventoryItem *Inv);

	/* Gets the head's location. For bot AI. */
	UFUNCTION(BlueprintCallable, Category = AI)
	FVector GetHeadLocation() const;

	/* Gets the torso's location. For bot AI. */
	UFUNCTION(BlueprintCallable, Category = AI)
	FVector GetTorsoLocation() const;

	virtual void TurnOff() override;

	virtual void SetWeaponFiringAllowed(bool bInWeaponFiringAllowed);

	/** Adds the player's starting inventory. **/
	virtual void SpawnInitialInventory(TArray<TSubclassOf<class AInventoryItem>> InventoryFromGameMode, bool bUsePawnDefaultInventory = false);

	// Creates and returns new item of class NewItemClass and adds it to the inventory.
	class AInventoryItem* CreateNewInventoryItem(TSubclassOf<class AInventoryItem> NewItemClass);

	/* Called by the player's previous weapon once it has finished being unequipped. */
	void OnWeaponUnequipFinish(class AInventoryItem* OldWeapon);

	// TEST: Anim sequence when idle.
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimSequence* IdleAnimSeq;

	// TEST: Default idle anim (i.e. when no weapon is equipped).
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimSequence* DefaultIdleAnimSeq;

	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimSequence* GetIdleAnimSequence() const;

	// Checks if the player can grab the pickup.
	// TODO: Maybe split to see if player:
	//    1. Can add item to inventory (current design), and:
	//    2. Can physically grab the item (too far, has no arms, etc.) 
	bool CanPickUpItem(class AInventoryItem* Item) const;

	virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime) override;

	/** Get the direction the pawn is currently aiming their held item. */
	virtual FRotator GetWeaponAimRot() const;

	/** Get the location where the player's aim starts (generally the location of their held item. */
	virtual FVector GetWeaponAimLoc() const;

	/** Calculates a matrix that represents where the pawn's held weapon is in the world. */
	virtual FMatrix GetWeaponAimMatrix() const;

	/* Offset the weapon from the view direction based on breathing, movement, etc. */
	void AddWeaponSway(float DeltaSeconds);

	// Float to track the weapon sway offset time.
	float WeaponSwayTime;

	// Float to track the weapon sway offset time due to breathing.
	float BreathingTime;

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

	// Overridden to add falling damage and other effects.
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

	/** Player's normal health amount, generally 100. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float FullHealth;

	/** Player's maximum health, generally 100. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float MaxHealth;

	/** Health that will not regenerate normally. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float SevereDamage;

	/** Default scalar for percentage of incoming damage that is considered "severe". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float SevereDamageScalar;

	/** Player's health regeneration rate per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float HealthRegenRate;

	/** Times to regenerate health per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float HealthTicksPerSec;

	// Timer and function to regenerate health when damaged.
	FTimerHandle TimerHandle_RegenHealth;

	// Renerate some health up to max health (minus severe damage).
	virtual void RegenHealth();

	/** Health of the player's legs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float LegHealth;

	/** Player's maximum health, generally 100. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
	float FullLegHealth;

	/** Currently equipped weapon. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_EquippedItem)
	class AInventoryItem* EquippedItem;

	/** Weapon we want to equip. */
	class AInventoryItem* PendingItem;

	UFUNCTION()
	void OnRep_EquippedItem(class AInventoryItem* LastItem);

	void SetEquippedWeapon(class AInventoryItem* NewWeapon, class AInventoryItem* LastWeapon = nullptr);

	/* Sets which weapon the player desires to equip. */
	void SetPendingWeapon(class AInventoryItem* NewWeapon);

	/** Mesh material instances. Created so we can manipulate colour, cloaking, etc. **/
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** Mesh material instances. Created so we can manipulate colour, cloaking, etc. **/
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> Mesh1PMIDs;

	/** Switches to the previous weapon in the inventory. */
	void OnPrevWeapon();

	/** Switches to the next weapon in the inventory. */
	void OnNextWeapon();

	/** Starts aiming weapon. */
	void OnStartAim();

	/** Stops aiming weapon. */
	void OnStopAim();

	/** Starts sprinting. */
	void OnStartSprint();

	/** Stops sprinting. */
	void OnStopSprint();

	/** Drops current weapon. */
	UFUNCTION(BlueprintCallable, Category = Input)
	void OnDropWeapon();

	/** Starts zoom. */
	UFUNCTION(BlueprintCallable, Category = Input)
	virtual void OnStartZoom();

	/** Stops zoom. */
	UFUNCTION(BlueprintCallable, Category = Input)
	virtual void OnStopZoom();

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

	// The default weapon classes to spawn. Empty for most multiplayer pawns.
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class AInventoryItem>> DefaultInventoryClasses;

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

	UFUNCTION(reliable, server, WithValidation)
	void ServerSetSprinting(bool bNewSprinting);

	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon(class AInventoryItem* Weapon);

	UFUNCTION(reliable, server, WithValidation)
	void ServerDropInventory(class AInventoryItem* Inv);

	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleUse();

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

	/** Updates the weapon's location and rotation relative to camera. **/
	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

private:

	/** Is the character trying to aim? **/
	UPROPERTY(Transient, Replicated)
	bool bIsAiming;

	/* Can player fire weapons? */
	bool bWeaponFiringAllowed;

	/** The current position of the pawn's weapon, from fully aimed (1) to fully unaimed (0). **/
	UPROPERTY(Transient, Replicated)
	float CurrentAimPct;

	/** Is the character sprinting? **/
	UPROPERTY(Transient, Replicated)
	bool bIsSprinting;

	/** Is the character zooming in their camera? */
	bool bIsZooming;

	/** The time to zoom the camera in. */
	UPROPERTY()
	float ZoomInTime;

	/** The time to zoom the camera out. */
	UPROPERTY()
	float ZoomOutTime;

	/** The magnification level when zoomed. */
	UPROPERTY()
	float ZoomScale;

	/** The percentage the character is zoomed in, 0 being no zoom and 1 being full zoom. */
	float CurrentZoomPct;

	FRotator WeaponRotationOffset;

	FVector WeaponLocationOffset;

	//////////////////////////////////////////////////////////////////////////
	// RECOIL STUFF!
	//  This section will be in constant flux as we test things out.

	/** The weapon's offset relative to the character's view direction. */
	FRotator HeldWeaponOffset;

	/* Last control rotation when we processed free aim.
		Needed to properly apply free aim. */
	FRotator LastControlRotation;

	/* Maximum radius the character's weapon can be offset by free aim. */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxFreeAimRadius;

public:
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

	UPROPERTY(EditAnywhere, Category = Temp)
	FVector TEMP_AnimOffset;
};

