// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Teleporter.h"
#include "SolCharacter.h"
#include "Player/SolAIController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Runtime/Engine/Classes/AI/NavigationSystemBase.h"

// Sets default values
ATeleporter::ATeleporter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	CapsuleComponent = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, "CapsuleComponent");
	CapsuleComponent->InitCapsuleSize(50.f, 100.f);
	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnOverlapBegin);
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &ATeleporter::OnOverlapEnd);
	CapsuleComponent->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//CapsuleComponent->SetupAttachment(GetRootComponent());
	RootComponent = CapsuleComponent;

	NavLinkComponent = CreateDefaultSubobject<UNavLinkComponent>(TEXT("NavLinkComponent"));
	NavLinkComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATeleporter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATeleporter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATeleporter::OnOverlapBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	// Consider teleporting only if we're not currently on the ignore list.
	if (ActorsToNotTeleport.Remove(OtherActor) == 0)
	{
		ASolCharacter* SChar = Cast<ASolCharacter>(OtherActor);
		if (SChar && ExitPoint != FVector::ZeroVector)
		{
			// TODO: ExitRotation seems to have no effect on DestRotator.
			FVector OriginVector = OtherActor->GetActorLocation();
			FVector DestVector = this->GetActorLocation() + ExitPoint;
			FRotator DestRotator = SChar->GetActorRotation() + ExitRotation;
			// Get rid of pitch/roll for characters.
			DestRotator.Pitch = 0.f;
			DestRotator.Roll = 0.f;

			// If any teleporters are at our destination, ensure they ignore our arrival (lest we enter an infinite loop).
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			TArray<AActor*> ActorsToIgnore;
			TArray<AActor*> OutActors;
			UKismetSystemLibrary::CapsuleOverlapActors(this, DestVector, SChar->GetCapsuleComponent()->GetScaledCapsuleRadius(), SChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), ObjectTypes, ATeleporter::GetClass(), ActorsToIgnore, OutActors);
			TArray<ATeleporter*> DestTeleporters;
			for (int i = 0; i < OutActors.Num(); i++)
			{
				ATeleporter* DestTeleporter = Cast<ATeleporter>(OutActors[i]);
				if (DestTeleporter)
				{
					DestTeleporters.Add(DestTeleporter);
					DestTeleporter->ActorsToNotTeleport.Add(SChar);
				}
			}

			// TODO: Any AI upon start teleporting.

			bool bTeleported = SChar->TeleportTo(DestVector, DestRotator);
			if (bTeleported)
			{
				if (EnterSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, EnterSound, OriginVector);
				}
				if (ExitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ExitSound, DestVector);
				}
				// TODO: Any AI upon successful teleporting.
			}
			// If teleport failed, don't let destination teleporters ignore us.
			else
			{
				for (int i = 0; i < DestTeleporters.Num(); i++)
				{
					DestTeleporters[0]->ActorsToNotTeleport.Remove(SChar);
				}
				// TODO: Any AI for failed teleport.
			}
		}
	}
}

void ATeleporter::OnOverlapEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ActorsToNotTeleport.Remove(OtherActor);
}

UCapsuleComponent* ATeleporter::GetCapsuleComponent()
{
	return CapsuleComponent;
}

void ATeleporter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateNavLinks();
}

#if WITH_EDITORONLY_DATA
void ATeleporter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateNavLinks();
}
#endif // WITH_EDITORONLY_DATA

void ATeleporter::UpdateNavLinks()
{
	if (NavLinkComponent)
	{
		check(NavLinkComponent->Links.Num() == 1);
		NavLinkComponent->Links[0].Left = FVector::ZeroVector;
		NavLinkComponent->Links[0].Right = ExitPoint;

		auto World = GetWorld();
		if (World)
		{
			auto NavSystem = World->GetNavigationSystem();
			if (NavSystem)
			{
				FNavigationSystem::UpdateComponentData(*NavLinkComponent);
			}
		}
	}
}
