// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "InventoryItem.h"
#include "PickupSpawner.generated.h"

UCLASS()
class SOL_API APickupSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupSpawner(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned.
	virtual void BeginPlay() override;
	
	// Called every frame.
	virtual void Tick( float DeltaSeconds ) override;

	// Classes of pickup to spawn, chosen randomly.
	UPROPERTY(EditAnywhere, Category = Gameplay)
	TArray<TSubclassOf<class AInventoryItem>> InventoryClasses;

	// Time between weapon spawns.
	UPROPERTY(EditAnywhere, Category = Gameplay)
	float RespawnTime;

	// If true, weapon won't spawn at game start; it'll spawn after its RespawnTime.
	UPROPERTY(EditAnywhere, Category = Gameplay)
	bool bDelayedSpawn;

	// Sound played when pickup is spawned.
	UPROPERTY(EditAnywhere, Category = Gameplay)
	USoundCue* RespawnSound;

	// Current list of spawned pickups.
	TArray<APickup*> SpawnedPickups;

	// Maximum amount of pickups that can exist before the spawner stops spawning them. Generally just one.
	UPROPERTY(EditAnywhere, Category = Gameplay)
	int32 MaxSpawnedPickups;

	// Called by spawned pickups upon being taken or destroyed.
	virtual void OnPickupDestroyed(APickup* LostItem);

protected:

	UFUNCTION(reliable, server, WithValidation)
	void ServerCreateNewPickup();
	bool ServerCreateNewPickup_Validate();
	void ServerCreateNewPickup_Implementation();

	// Last time a weapon was spawned.
	float NextSpawnTime;
};
