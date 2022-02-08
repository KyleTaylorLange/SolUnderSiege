// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Bullet.h"

ABullet::ABullet(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundDefaultFX(TEXT("/Game/ShooterGameEffects/ParticleSystems/Weapons/AssaultRifle/Impacts/P_AssaultRifle_IH.P_AssaultRifle_IH"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundPlayerFX(TEXT("/Game/ShooterGameEffects/ParticleSystems/Gameplay/Player/P_body_bullet_impact.P_body_bullet_impact"));
	static ConstructorHelpers::FObjectFinder<USoundCue> FoundDefaultIS(TEXT("/Game/Sounds/SC_BulletImpact.SC_BulletImpact"));
	static ConstructorHelpers::FObjectFinder<USoundCue> FoundPlayerIS(TEXT("/Game/Sounds/SC_InfantryImpact.SC_InfantryImpact"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundPS(TEXT("/Game/Blueprints/PS_BulletTest.PS_BulletTest"));

	ParticleSysComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleSysComp"));
	ParticleSysComp->SetTemplate(FoundPS.Object);
	ParticleSysComp->AttachTo(GetRootComponent());

	// The below speeds are for testing purposes only. 
	// Remember, 10 000 cm/s = 100 m/s, so the values are huge.
	ProjectileMovement->InitialSpeed = 2500.f; //25000.f;
	ProjectileMovement->MaxSpeed = 2500.f; //100000.f; //25000.f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	DefaultImpactFX = FoundDefaultFX.Object;
	DefaultImpactSound = FoundDefaultIS.Object;
	PlayerImpactFX = FoundPlayerFX.Object;
	PlayerImpactSound = FoundPlayerIS.Object;

	SizeDamageReferencePoint = 50.f;
	BulletDrawColor = FLinearColor::White;

	bDetonatesOnImpact = true;
	InitialLifeSpan = 5.0f;

	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	OriginPoint = this->GetActorLocation();
	CalculateBulletSize();
}

void ABullet::CalculateBulletSize()
{
	float GirthScale = 0.7f;
	float LengthScale = 0.7f;
	float PseudoArea = 1.0f;
	float SizeDamageScalar = ImpactDamage / SizeDamageReferencePoint;
	
	if (PrimitiveComp)
	{
		USphereComponent* SphereComp = Cast<USphereComponent>(PrimitiveComp);
		if (SphereComp)
		{
			PseudoArea = FMath::Square(SphereComp->GetScaledSphereRadius() * 2.f);
			GirthScale *= SphereComp->GetScaledSphereRadius() * 2.f;
		}
	}

	LengthScale *= SizeDamageScalar / PseudoArea;

	if (ParticleSysComp)
	{
		ParticleSysComp->SetWorldScale3D(FVector(LengthScale, GirthScale, GirthScale));
	}
}

void ABullet::SetBulletColor(FLinearColor InColor)
{
	const FVector VectorizedColor = FVector(InColor.R, InColor.G, InColor.B);
	if (ParticleSysComp)
	{
		ParticleSysComp->SetVectorParameter("BulletColour", VectorizedColor);
		//ParticleSysComp->SetVectorParameter("BulletSize", FVector(100.1f));
	}
}

void ABullet::SetBulletDiameter(float InDiameter)
{
	if (PrimitiveComp)
	{
		USphereComponent* SphereComp = Cast<USphereComponent>(PrimitiveComp);
		if (SphereComp)
		{
			SphereComp->InitSphereRadius(InDiameter * 0.05f);
			CalculateBulletSize();
		}
		if (ParticleSysComp)
		{
			float GlobalScale = 0.1f;
			ParticleSysComp->SetVectorParameter("BulletSize", FVector(InDiameter * GlobalScale));
		}
	}
}

void ABullet::SetBulletSpeed(float InSpeedMPS)
{
	if (ProjectileMovement)
	{
		const float TempBulletSpeedScalar = 1.f;
		ProjectileMovement->InitialSpeed = 100.f * InSpeedMPS * TempBulletSpeedScalar;
		ProjectileMovement->MaxSpeed = ProjectileMovement->InitialSpeed;
	}
}

void ABullet::SetBulletProperties(FBulletProperties InProps)
{
	BulletProperties = InProps;
	SetBulletColor(BulletProperties.Color);
	SetBulletDiameter(BulletProperties.Diameter);
	SetBulletSpeed(BulletProperties.Speed);
}
