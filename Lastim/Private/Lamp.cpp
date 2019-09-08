// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Lamp.h"


// Sets default values
ALamp::ALamp(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	LightColor = FLinearColor(1.0f, 0.25f, 1.0f);
	
	LampMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "Mesh");
	LampPL = ObjectInitializer.CreateDefaultSubobject<UPointLightComponent>(this, "PointLight");
	LampPL->SetIntensity(LightPower * 2500);
	LampPL->SetLightColor(LightColor);
	
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALamp::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALamp::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

