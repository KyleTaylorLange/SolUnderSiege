// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "UsableObjectInterface.h"
#include "DominationControlPoint.generated.h"

DECLARE_DELEGATE_OneParam(FOnScoreCP, ADominationControlPoint*)

UCLASS()
class LASTIM_API ADominationControlPoint : public AActor, public IUsableObjectInterface
{
	GENERATED_UCLASS_BODY()
	
public:	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Gets owning player.
	class ASolPlayerState* GetOwningPlayer();

	// Gets owning team.
	class ATeamState* GetOwningTeam();

	virtual bool CanBeUsedBy(AActor* User) override;

	virtual bool OnStartUseBy(AActor* User) override;

	virtual FString GetUseActionName(AActor* User) override;

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
