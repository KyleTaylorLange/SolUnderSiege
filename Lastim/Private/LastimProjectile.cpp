// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimCharacter.h"
#include "LastimProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

ALastimProjectile::ALastimProjectile(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	// By default, use a sphere for collision.
	PrimitiveComp = ObjectInitializer.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	PrimitiveComp->BodyInstance.SetCollisionProfileName("Projectile");
	PrimitiveComp->OnComponentHit.AddDynamic(this, &ALastimProjectile::OnImpact);

	USphereComponent* SphereComp = Cast<USphereComponent>(PrimitiveComp);
	if (SphereComp)
	{
		SphereComp->InitSphereRadius(1.0f);
	}

	// Players can't walk on it
	PrimitiveComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	PrimitiveComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = PrimitiveComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = PrimitiveComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = false; // true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 5 seconds by default
	//InitialLifeSpan = 5.0f;
	bDetonated = false;
	bDetonatesOnImpact = false;
	bReplicates = true;
	bReplicateMovement = true;
}

void ALastimProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//PrimitiveComp->MoveIgnoreActors.Add(Instigator);
}

void ALastimProjectile::InitVelocity(FVector& ShootDirection)
{
	if (ProjectileMovement)
	{
		// TEST: Pawn's velocity is ShootDirection. Try to inherit velocity.
		ProjectileMovement->Velocity += ShootDirection; //ShootDirection * ProjectileMovement->InitialSpeed;
	}
}

void ALastimProjectile::OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (Role == ROLE_Authority)
	{
		if (bDetonatesOnImpact && !bDetonated)
		{
			Detonate(Hit);
		}
		else
		{
			//Impact(OtherActor, OtherComp, Hit);
		}
	}
}

void ALastimProjectile::Detonate(const FHitResult Hit)
{
	// Convert to PointDamageEvent for localized damage.
	FPointDamageEvent PDEvent;
	PDEvent.HitInfo = Hit;

	const FVector NudgedImpactLocation = Hit.ImpactPoint + Hit.ImpactNormal * 10.0f;

	FVector ProjDirection = GetActorRotation().Vector();
	const FVector StartTrace = NudgedImpactLocation - ProjDirection * 25;
	const FVector EndTrace = NudgedImpactLocation + ProjDirection * 25;

	static FName ProjTag = FName(TEXT("ProjClient"));

	FCollisionQueryParams TraceParams(ProjTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;
	//AActor HitObject = Cast<ALastimCharacter>(Hit.Actor);
	//TraceParams.AddIgnoredComponents()

	FHitResult TestImpact;
	bool bTestBool = GetWorld()->LineTraceSingleByChannel(TestImpact, StartTrace, EndTrace, COLLISION_PROJECTILE, TraceParams);
	PDEvent.HitInfo.PhysMaterial = TestImpact.PhysMaterial.Get();

	// Try another trace towards the actor if we didn't get a material on our first try.
	// CURRENTLY CRASHES GAME
	/*
	if (PDEvent.HitInfo.PhysMaterial.Get() == NULL)
	{
		FHitResult NewTestImpact;
		const FVector SecondEndTrace = Hit.Actor->GetActorLocation();
		GetWorld()->LineTraceSingleByChannel(NewTestImpact, StartTrace, SecondEndTrace, COLLISION_PROJECTILE, TraceParams);
		PDEvent.HitInfo.PhysMaterial = NewTestImpact.PhysMaterial.Get();
	}
	*/
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("HitPhysMat: %s"), *PDEvent.HitInfo.PhysMaterial->GetClass() ));

	if ( (ImpactDamage > 0) && (Hit.Actor != NULL) && (Hit.Actor != this) ) //&& DamageTypeClass)
	{
		// Quick workaround to prevent killing ourselves.
		if (Hit.Actor != Instigator)
		{
			Hit.Actor->TakeDamage(ImpactDamage, PDEvent, Instigator->Controller, this);

			// Disabled for now. (causes Pickup locations to be out of synch.)
			if (Hit.Component != NULL && Hit.Component->IsSimulatingPhysics())
			{
				Hit.Component->AddImpulseAtLocation(GetVelocity().GetSafeNormal() * ImpactDamage * 200.0f, GetActorLocation());
			}
			SpawnImpactEffects(PDEvent.HitInfo);
		}
	}
	if (ExplosionDamage > 0 && ExplosionRadius > 0) //&& DamageTypeClass)
	{
		SpawnExplosionEffects();
		//UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, NudgedImpactLocation, ExplosionRadius, DamageTypeClass, TArray<AActor*>(), this, GetInstigatorController() );
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, DamageTypeClass, TArray<AActor*>(), this, GetInstigatorController());
	}
	SpawnImpactEffects(PDEvent.HitInfo);
	bDetonated = true;

	// TO "DisableAndDestroy()"
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}
	if (PrimitiveComp)
	{
		PrimitiveComp->SetHiddenInGame(true);
		PrimitiveComp->SetVisibility(false);
		PrimitiveComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	if (ParticleSysComp)
	{
		ParticleSysComp->Deactivate();
	}
	
	SetLifeSpan(2.0);
}

/*
bool ALastimProjectile::Impact(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	return true;
}
*/

void ALastimProjectile::Impact(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult Hit)
{
	ImpactCount++;
}

void ALastimProjectile::OnRep_Detonated()
{
	FVector ProjDirection = GetActorRotation().Vector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;

	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(TEXT("ProjClient"), true, Instigator)))
	{
		// failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}
	
	Detonate(Impact);
}

/*
void ALastimProjectile::OnRep_ImpactCount(AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Impact(OtherActor, OtherComp, NormalImpulse, Hit);
}
*/

// Commented out parts taken from ShooterGame.
// It has a very nice implementation using a "ShooterImpactEffect" class to handle determining the surface type
//   and subsequent effects to spawn, but for now all we care about is if we hit a player.
void ALastimProjectile::SpawnImpactEffects(const FHitResult& Impact)
{
	// Establish default effects.
	USoundCue* ImpactSound = DefaultImpactSound;
	UParticleSystem* ImpactFX = DefaultImpactFX;

	UPhysicalMaterial* HitPhysMat = Impact.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	//SurfaceType1 = Flesh
	if (HitSurfaceType == SurfaceType1 && PlayerImpactSound)
	{
		ImpactSound = PlayerImpactSound;
	}
	if (HitSurfaceType == SurfaceType1 && PlayerImpactFX)
	{
		ImpactFX = PlayerImpactFX;
	}

	// Play effects.
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		}
		if (ImpactFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, GetActorLocation(), GetActorRotation());
		}
	}
	
	
	/*
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		FHitResult UseImpact = Impact;

		// trace again to find component lost during replication
		if (!Impact.Component.IsValid())
		{
			const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
			const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
			FHitResult Hit = WeaponTrace(StartTrace, EndTrace);
			UseImpact = Hit;
		}

		AShooterImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AShooterImpactEffect>(ImpactTemplate, Impact.ImpactPoint, Impact.ImpactNormal.Rotation());
		if (EffectActor)
		{
			EffectActor->SurfaceHit = UseImpact;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint));
		}
	}
	*/
}

void ALastimProjectile::SpawnExplosionEffects()
{
	// Establish default effects.
	USoundCue* ExplosionSound = DefaultExplosionSound;
	UParticleSystem* ExplosionFX = DefaultExplosionFX;

	//UPhysicalMaterial* HitPhysMat = Impact.PhysMaterial.Get();
	//EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	//SurfaceType1 = Flesh
	/**
	if (HitSurfaceType == SurfaceType1 && PlayerImpactSound)
	{
		ImpactSound = PlayerImpactSound;
	}
	if (HitSurfaceType == SurfaceType1 && PlayerImpactFX)
	{
		ImpactFX = PlayerImpactFX;
	}
	*/

	// Play effects.
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());
		}
		if (ExplosionFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation(), GetActorRotation());
		}
	}
}

float ALastimProjectile::GetTimeToLocation(const FVector& StartLoc, const FVector& EndLoc) const
{
	const float Distance = (StartLoc - EndLoc).Size();
	if (ProjectileMovement)
	{
		// For now, just do this. 
		return Distance / ProjectileMovement->InitialSpeed;
	}
	return 0.0f;
}

void ALastimProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALastimProjectile, bDetonated);
	//DOREPLIFETIME(ALastimProjectile, ImpactCount);
}