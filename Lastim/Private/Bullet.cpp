// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Bullet.h"

ABullet::ABullet(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundDefaultFX(TEXT("/Game/ShooterGameEffects/ParticleSystems/Weapons/AssaultRifle/Impacts/P_AssaultRifle_IH.P_AssaultRifle_IH"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundPlayerFX(TEXT("/Game/ShooterGameEffects/ParticleSystems/Gameplay/Player/P_body_bullet_impact.P_body_bullet_impact"));
	static ConstructorHelpers::FObjectFinder<USoundCue> FoundDefaultIS(TEXT("/Game/Sounds/SC_BulletImpact.SC_BulletImpact"));
	static ConstructorHelpers::FObjectFinder<USoundCue> FoundPlayerIS(TEXT("/Game/Sounds/SC_InfantryImpact.SC_InfantryImpact"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundPS(TEXT("/Game/Blueprints/PS_BulletTest.PS_BulletTest"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundTrailFX(TEXT("/Game/ShooterGameEffects/ParticleSystems/Weapons/AssaultRifle/Muzzle/P_AssaultRifle_Trail.P_AssaultRifle_Trail"));

	/* Removing the regular PSC, replaced with a trail that can outlast the projectile. */
	//ParticleSysComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleSysComp"));
	//ParticleSysComp->SetTemplate(FoundPS.Object);
	//ParticleSysComp->AttachTo(GetRootComponent());
	BulletTrailFX = FoundTrailFX.Object;
	BulletTrailPSC = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("BulletTrailPSC"));
	BulletTrailPSC->SetTemplate(BulletTrailFX);
	BulletTrailPSC->SetVectorParameter("ShockBeamEnd", this->GetActorLocation());
	BulletTrailPSC->SetVectorParameter("ShockBeamStart", this->GetActorLocation());

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

// Called every frame.
void ABullet::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BulletTrailPSC)
	{
		BulletTrailPSC->SetVectorParameter("ShockBeamEnd", this->GetActorLocation());
		BulletTrailPSC->SetVectorParameter("ShockBeamStart", (OriginPoint + this->GetActorLocation()) * 0.5f);
	}
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
	if (BulletTrailPSC)
	{
		BulletTrailPSC->SetWorldScale3D(FVector(LengthScale, FMath::Square(GirthScale), FMath::Square(GirthScale)));
	}
}

void ABullet::SetBulletColor(FLinearColor InColor)
{
	const FVector VectorizedColor = FVector(InColor.R, InColor.G, InColor.B);
	if (ParticleSysComp)
	{
		ParticleSysComp->SetVectorParameter("BulletColour", VectorizedColor);
	}
	if (BulletTrailPSC)
	{
		BulletTrailPSC->SetVectorParameter("BulletColour", VectorizedColor);
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
	}
}

void ABullet::SetBulletSpeed(float InSpeedMPS)
{
	if (ProjectileMovement)
	{
		const float TempBulletSpeedScalar = 1.f; // 0.25f;
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