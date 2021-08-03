// Copyright Kyle Taylor Lange
#pragma once
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS(config=Game)
class AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	/** Called when projectile impacts a surface. */
	UFUNCTION()
	void OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	// Component used for collision. May be a mesh, or just a collision shape.
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class UPrimitiveComponent* PrimitiveComp;

	// Particle system component. Not initalized in the base class, but here for subclasses that need it.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Projectile)
	class UParticleSystemComponent* ParticleSysComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement) //, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* ProjectileMovement;

	/** Damage upon contacting an actor or exploding, and the radius of an explosion. 
	    Firearm subclasses change these values before spawning. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ImpactDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ExplosionDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ExplosionRadius;

	/** Inherits the velocity of the owner, adding their velocity to the projectile's velocity. */
	void InheritVelocity(FVector& OwnerVelocity);

	/* Gets the time the projectile requires to reach a target. */
	virtual float GetTimeToLocation(const FVector& StartLoc, const FVector& EndLoc) const;

	TSubclassOf<UDamageType> DamageTypeClass;

protected:

	// "Detonates" the object; exploding and/or dealing kinetic projectile damage.
	void Detonate(const FHitResult Hit);

	// Plays effects when touching a surface and surviving.
	//UFUNCTION(unreliable, server, WithValidation)
	void Impact(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult Hit);
	//bool Impact_Validate(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	//void Impact_Implementation(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Does the projectile deal damage/explode after impacting another object?
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	bool bDetonatesOnImpact;

	// Has the projectile detonated? Used for replication.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Detonated)
	bool bDetonated;

	// Amount of times object has impacted a surface. Used for replicate.
	//UPROPERTY(Transient, ReplicatedUsing = OnRep_ImpactCount)
	int32 ImpactCount;

	// Replicates the detonation event.
	UFUNCTION()
	void OnRep_Detonated();

	// Replicates the impact event.
	//UFUNCTION()
	//void OnRep_ImpactCount(AActor* OtherActor, UPrimitiveComponent* OtherComp, /*FVector NormalImpulse,*/ const FHitResult& Hit);

	// Creates the cosmetic impact effects.
	void SpawnImpactEffects(const FHitResult& Impact);

	// Creates the cosmetic explosion effects.
	void SpawnExplosionEffects();

	// NOTE/TODO: Should these be split off into an "ImpactEffect" class like in ShooterGame?
	//            It would make checking each physical material a lot easier.
	//            However, it isn't high priority, so I'm just dumping these here.

	// Sound played when impacting a surface.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	USoundCue* DefaultImpactSound;

	// Particle system played when impacting a surface.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* DefaultImpactFX;

	// Sound played when impacting a player.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	USoundCue* PlayerImpactSound;

	// Particle system played when impacting a player.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* PlayerImpactFX;

	// Sound played when exploding.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	USoundCue* DefaultExplosionSound;

	// Particle system played when exploding.
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* DefaultExplosionFX;
};

