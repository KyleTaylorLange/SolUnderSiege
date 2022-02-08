// Copyright Kyle Taylor Lange

#pragma once

#include "KineticProjectile.h"
#include "Bullet.generated.h"

USTRUCT()
struct FBulletProperties
{
	GENERATED_USTRUCT_BODY()

	/* Colour of the bullet's tracers and sparks. */
	UPROPERTY(EditDefaultsOnly, Category = Bullet)
	FLinearColor Color;

	/* Diameter of the bullet, in millimetres. */
	UPROPERTY(EditDefaultsOnly, Category = Bullet)
	float Diameter;

	/* Speed of the bullet, in metres per second. */
	UPROPERTY(EditDefaultsOnly, Category = Bullet)
	float Speed;

	/** defaults */
	FBulletProperties()
	{
		Color = FLinearColor::White;
		Diameter = 10.f;
		Speed = 250.f;
	}

	FBulletProperties(FLinearColor InColor, float InDiameter, float InSpeed)
	{
		Color = InColor;
		Diameter = InDiameter;
		Speed = InSpeed;
	}
};

/**
 * BULLET
 *  A special subclass of KineticProjectile that facilitates the creation of common "bulletlike" projectiles.
 *  This class will include setting colour, size, and physics properties.
 */
UCLASS()
class SOL_API ABullet : public AKineticProjectile
{
	GENERATED_BODY()
	
public:

	ABullet(const FObjectInitializer& ObjectInitializer);
	
	/* Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	float SizeDamageReferencePoint;

	/* Resizes projectile based on diameter and damage. */
	virtual void CalculateBulletSize();

	/* Set the bullet's visual colour. */
	virtual void SetBulletColor(FLinearColor InColor);

	/* Sets projectile diameter in millimetres. */
	virtual void SetBulletDiameter(float InDiameter);

	/* Sets the bullet's speed in metres per second. */
	virtual void SetBulletSpeed(float InSpeedMPS);

	/* Sets many bullet properties. */
	virtual void SetBulletProperties(FBulletProperties InProps);

private:

	/* Colour used for the bullet tracers and sparks. */
	FLinearColor BulletDrawColor;

	FBulletProperties BulletProperties;

	FVector OriginPoint;

	/* Particle system for the bullet trail. */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* BulletTrailFX;
};
