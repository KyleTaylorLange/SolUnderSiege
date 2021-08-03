// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SpecificPickup.h"
#include "InventoryItem.h"
#include "SolGameMode.h"
#include "PickupSpawner.h"


// Sets default values
APickupSpawner::APickupSpawner(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Apparently GetActorLocation() uses the RootComponent, so we need to have something.
	// We need some type of component as the root for this thing to spawn anywhere but the map origin.
	// TODO: Make an editor-only 3D mesh or sprite.

	UCapsuleComponent* CapsuleComp = ObjectInitializer.CreateDefaultSubobject<UCapsuleComponent>(this, TEXT("CapsuleComp"));
	CapsuleComp->InitCapsuleSize(25.0f, 25.0f);
	//CapsuleComp->bDrawOnlyIfSelected = true;
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleComp->AlwaysLoadOnServer = true;
	RootComponent = CapsuleComp;

	RespawnTime = 30.f;
	bDelayedSpawn = false;
	NextSpawnTime = 0.f;
	MaxSpawnedPickups = 1;
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawner::BeginPlay()
{
	Super::BeginPlay();
	// Allow the game mode to modify the list of items we can spawn.
	if (GetWorld() && GetWorld()->GetAuthGameMode<ASolGameMode>())
	{
		GetWorld()->GetAuthGameMode<ASolGameMode>()->InitPickupSpawner(this);
	}
	// Spawn our first pickup or set a timer for it.
	if (!bDelayedSpawn)
	{
		ServerCreateNewPickup();
	}
	else
	{
		NextSpawnTime = RespawnTime;
	}
}

void APickupSpawner::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	// Spawn a pickup if we are due.
	if (NextSpawnTime < GetWorld()->GetTimeSeconds() && MaxSpawnedPickups > SpawnedPickups.Num() )
	{
		ServerCreateNewPickup();
	}
}

bool APickupSpawner::ServerCreateNewPickup_Validate()
{
	return true;
}

void APickupSpawner::ServerCreateNewPickup_Implementation()
{
	// Pick an item to spawn.
	TSubclassOf<AInventoryItem> ClassToSpawn = nullptr;
	if (InventoryClasses.Num() > 0)
	{
		ClassToSpawn = InventoryClasses[FMath::RandRange(0, InventoryClasses.Num() - 1)];
	}
	// Allow the game mode to modify our choice.
	if (GetWorld() && GetWorld()->GetAuthGameMode<ASolGameMode>())
	{
		GetWorld()->GetAuthGameMode<ASolGameMode>()->ModifyPickupToSpawn(ClassToSpawn);
	}
	// If we have something to spawn, spawn it!
	if (ClassToSpawn)
	{
		// Make a new inventory item.
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// Allow the game mode to modify our default list of items we can spawn.
		AInventoryItem* NewInvItem = GetWorld()->SpawnActor<AInventoryItem>(ClassToSpawn, SpawnInfo);

		// Make a new pickup.
		FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 100.f); // Temporarily increase it so it's further away from the ground.
		FRotator SpawnRotation = GetActorRotation();
		FTransform SpawnTM(SpawnRotation, SpawnLocation);
		ASpecificPickup* NewPickup = Cast<ASpecificPickup>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ASpecificPickup::StaticClass(), SpawnTM));
		if (NewPickup && NewInvItem)
		{
			NewPickup->AssignItemToPickup(NewInvItem);
			//int32 Index = FMath::RandRange(0, InventoryClasses.Num() - 1);
			//NewPickup->SetInventoryClass(InventoryClasses[Index]);
			NewPickup->Spawner = this;
			NewPickup->SetLifeSpan(300.f); // Temporary, in case it fell out of the map.
			SpawnedPickups.Add(NewPickup);

			FVector WeaponOrigin;
			FVector WeaponExtent = FVector::ZeroVector;

			NewPickup->GetActorBounds(true, WeaponOrigin, WeaponExtent);
			////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s"), *WeaponExtent.ToString()));
			SpawnTM += FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 5.0f + (2 * WeaponExtent.Z)));

			NextSpawnTime = GetWorld()->GetTimeSeconds() + RespawnTime;
			UGameplayStatics::FinishSpawningActor(NewPickup, SpawnTM);
			if (GetNetMode() != NM_DedicatedServer && RespawnSound)
			{
				////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Black, FString::Printf(TEXT("Spawn Location: %s"), *SpawnTM.GetTranslation().ToString()));
				UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, SpawnTM.GetTranslation());
			}
		}
	}
}

void APickupSpawner::OnPickupDestroyed(APickup* LostItem)
{
	// If the spawner is full, set a new respawn time to prevent an immediate respawn.
	if (MaxSpawnedPickups >= SpawnedPickups.Num())
	{
		NextSpawnTime = GetWorld()->GetTimeSeconds() + RespawnTime;
	}
	SpawnedPickups.Remove(LostItem);
}
