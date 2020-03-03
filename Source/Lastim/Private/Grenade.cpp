// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Grenade.h"

AGrenade::AGrenade(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDetonatesOnImpact = false;
	//InitialLifeSpan = 5.0f;
	FuseTime = 2.5f;
}

void AGrenade::BeginPlay()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		//Roll the dice
		//RNGStream.GenerateNewSeed();
		//Seed = RNGStream.GetCurrentSeed();

		//Set the fuse timer
		//float ExplodeTime = FuseTime + FMath::FRand() * RandomFuseMod;
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AGrenade::FuseTimer, FuseTime, true);
	}

	Super::BeginPlay();
}

void AGrenade::FuseTimer()
{
	FHitResult FakeHit;
	
	Detonate(FakeHit);
}