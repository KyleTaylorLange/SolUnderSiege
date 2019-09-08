// Copyright Kyle Taylor Lange

#pragma once

#include "GameFramework/Actor.h"
#include "Lamp.generated.h"

UCLASS()
class LASTIM_API ALamp : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALamp(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// The static mesh component used by this lamp.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lamp")
	UStaticMeshComponent* LampMesh;

	// The static mesh used by this lamp component.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lamp")
	UPointLightComponent* LampPL;

	//The light's colour and strength; used for both the PointLight and the emissive texture.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp")
	FLinearColor LightColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lamp")
	float LightPower;
	
};
