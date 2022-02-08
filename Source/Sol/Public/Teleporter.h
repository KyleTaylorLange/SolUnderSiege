// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/NavigationSystem/Public/NavLinkComponent.h"
#include "UObject/UnrealType.h"
#include "Teleporter.generated.h"

UCLASS()
class SOL_API ATeleporter : public AActor
{
	GENERATED_UCLASS_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATeleporter();

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult);

	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITORONLY_DATA

	UFUNCTION()
	UCapsuleComponent* GetCapsuleComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere)
	class UNavLinkComponent* NavLinkComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	ATeleporter* ExitTeleporter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MakeEditWidget = ""))
	FVector ExitPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MakeEditWidget = ""))
	FRotator ExitRotation;

	// List of actors to not teleport upon touch (likely because they are entering us).
	TArray<AActor*> ActorsToNotTeleport;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundCue* EnterSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	class USoundCue* ExitSound;

private:
	UFUNCTION()
	void OnConstruction(const FTransform& Transform) override;

	UFUNCTION()
	void UpdateNavLinks();

};
