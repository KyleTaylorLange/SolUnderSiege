// Copyright Kyle Taylor Lange

#pragma once

#include "LastimDestructible.h"
#include "Door.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ADoor : public ALastimDestructible
{
	GENERATED_UCLASS_BODY()

public:

	// BEGIN REDUNDANT MESH VARIABLES!
	
	/** Meshes for this door. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TArray<class UStaticMeshComponent*> DoorMeshes;

	/** Mesh for this door. **/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Door")
	class UStaticMeshComponent* Mesh;

	/** Meshes for this door. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TArray<class UStaticMesh*> DoorStaticMeshes;

	// END REDUNDANT MESH VARIABLES!

	/** Opened and closed positions of the door. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TArray<FVector> OpenPositions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	TArray<FVector> ClosedPositions;

	/** Collision overlay for automatic door opening. **/
	UPROPERTY(VisibleAnywhere, Category = "Door")
	class UBoxComponent* OverlayBox;
	/** Amount of players currently overlapping the box. **/
	UPROPERTY(BlueprintReadWrite, Category = "Door")
	int32 PlayerOverlapCount;

	/** Called when someone enters and leaves the sphere component, respectively. **/
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Does door open if someone is near it? Used to activate by button or to relegate to another door. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic;
	/** List of doors to close and open with this one. **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ADoor*> OwnedDoors;

	/** Open and close the door, respectively. **/
	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual void OpenDoor();
	UFUNCTION(BlueprintCallable, Category = "Door")
	virtual void CloseDoor();

	/** Time it takes to open and close the door, respectively. **/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float OpenDuration;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CloseDuration;
	
};
