// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "SolPlayerState.h"
#include "TeamState.h"
#include "SolGameState.h"
#include "DominationControlPoint.h"


// Sets default values
ADominationControlPoint::ADominationControlPoint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UParticleSystem> FoundPS(TEXT("/Game/Effects/ParticleSystems/PS_DomCPSymbol.PS_DomCPSymbol"));
	
	BaseMeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("BaseMeshComp"));
	BaseMeshComp->SetupAttachment(GetRootComponent());
	BaseMeshComp->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	BaseMeshComp->CastShadow = true;
	BaseMeshComp->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	RootComponent = BaseMeshComp;

	LightColor = FLinearColor::White;
	SymbolPSComp = ObjectInitializer.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("SymbolPSComp"));
	SymbolPSComp->SetTemplate(FoundPS.Object);
	SymbolPSComp->SetupAttachment(GetRootComponent());
	SymbolPSComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	SymbolPSComp->CastShadow = true;
	SymbolPSComp->SetVectorParameter("LightColour", FVector(LightColor.R, LightColor.G, LightColor.B));

	LightComp = ObjectInitializer.CreateDefaultSubobject<UPointLightComponent>(this, "PointLight");
	LightComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	LightComp->CastDynamicShadows = true;
	LightComp->SetLightColor(LightColor);
	LightComp->SetIntensity(2500.f);
	LightComp->SetupAttachment(GetRootComponent());

	DetectionCapsule = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, "DetectionCapsule");
	DetectionCapsule->InitCapsuleSize(100.f, 50.f);
	DetectionCapsule->OnComponentBeginOverlap.AddDynamic(this, &ADominationControlPoint::OnOverlapBegin);
	DetectionCapsule->OnComponentEndOverlap.AddDynamic(this, &ADominationControlPoint::OnOverlapEnd);
	DetectionCapsule->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	DetectionCapsule->SetupAttachment(GetRootComponent());

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
}

// Called when the game starts or when spawned
void ADominationControlPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADominationControlPoint::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void ADominationControlPoint::OnOverlapBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	ASolCharacter* LChar = Cast<ASolCharacter>(OtherActor);
	if (LChar)
	{
		ASolPlayerState* PS = Cast<ASolPlayerState>(LChar->GetPlayerState());
		if (PS)
		{
			OnTeamChange(PS);
		}
	}
}

void ADominationControlPoint::OnOverlapEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	/* Nothing here yet. */
}

void ADominationControlPoint::OnTeamChange(ASolPlayerState* NewOwner)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		/* Change to player's team if the team doesn't already own this point. */
		if (NewOwner && NewOwner->GetTeam() && NewOwner->GetTeam() != OwningTeam)
		{
			OwningPlayer = NewOwner;
			OwningTeam = OwningPlayer->GetTeam();

			/* Reset the score timer. */
			GetWorldTimerManager().PauseTimer(TimerHandle_ScorePoint);
			GetWorldTimerManager().SetTimer(TimerHandle_ScorePoint, this, &ADominationControlPoint::AddPointToOwner, 5.0f, true);

			if (OwningTeam)
			{
				LightColor = OwningTeam->GetTeamColor();
			}
			else
			{
				LightColor = FLinearColor::White;
			}

			if (LightComp)
			{
				LightComp->SetLightColor(LightColor);
			}
			if (SymbolPSComp)
			{
				SymbolPSComp->SetVectorParameter("LightColour", FVector(LightColor.R, LightColor.G, LightColor.B));
			}
		}
	}
}

void ADominationControlPoint::AddPointToOwner()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (OwningTeam != nullptr)
		{
			OwningTeam->AddScore(1);
			if (OwningPlayer != nullptr)
			{
				OwningPlayer->AddScore(1);
			}
		}
	}
}

void ADominationControlPoint::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADominationControlPoint, OwningTeam);
	DOREPLIFETIME(ADominationControlPoint, OwningPlayer);
}

void ADominationControlPoint::OnRep_OwningTeam()
{
	if (OwningTeam)
	{
		LightColor = OwningTeam->GetTeamColor();
	}
	else
	{
		LightColor = FLinearColor::White;
	}

	if (LightComp)
	{
		LightComp->SetLightColor(LightColor);
	}
	if (SymbolPSComp)
	{
		SymbolPSComp->SetVectorParameter("LightColour", FVector(LightColor.R, LightColor.G, LightColor.B));
	}
}