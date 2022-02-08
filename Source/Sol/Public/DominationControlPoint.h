// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "DominationControlPoint.generated.h"

DECLARE_DELEGATE_OneParam(FOnScoreCP, ADominationControlPoint*)

UCLASS()
class SOL_API ADominationControlPoint : public AActor
{
	GENERATED_UCLASS_BODY()
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Gets owning player.
	class ASolPlayerState* GetOwningPlayer();

	// Gets owning team.
	class ATeamState* GetOwningTeam();

	UFUNCTION()
	virtual bool CanInteractWith(UInteractableComponent* Component, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction);

	UFUNCTION()
	virtual void OnStartUseBy(UInteractableComponent* Interactable, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction);

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FOnScoreCP OnScoreCPDelegate;



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
	UInteractableComponent* InteractableComponent;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	bool bCaptureOnTouch;

	virtual void OnTeamChange(class ASolPlayerState* NewOwner);

	FTimerHandle TimerHandle_ScorePoint;

	UPROPERTY(Transient, Replicated)
	class ASolPlayerState* OwningPlayer;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_OwningTeam)
	class ATeamState* OwningTeam;

	UFUNCTION()
	virtual void OnRep_OwningTeam();

	UFUNCTION()
	virtual void AddPointToOwner();
};
