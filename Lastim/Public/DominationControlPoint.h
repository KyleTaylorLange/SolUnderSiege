// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "DominationControlPoint.generated.h"

UCLASS()
class LASTIM_API ADominationControlPoint : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


private:

	UPROPERTY(EditAnywhere, Category = Mesh)
	UStaticMeshComponent* BaseMeshComp;

	UPROPERTY(EditAnywhere, Category = Mesh)
	UParticleSystemComponent* SymbolPSComp;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UPointLightComponent* LightComp;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UCapsuleComponent* DetectionCapsule;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FLinearColor LightColor;

	virtual void OnTeamChange(class ALastimPlayerState* NewOwner);

	FTimerHandle TimerHandle_ScorePoint;

	UPROPERTY(Transient, Replicated)
	class ALastimPlayerState* OwningPlayer;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_OwningTeam)
	class ATeamState* OwningTeam;

	UFUNCTION()
	virtual void OnRep_OwningTeam();

	UFUNCTION()
	virtual void AddPointToOwner();
};
