// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimCharacter.h"
#include "ThrowableWeapon.h"

AThrowableWeapon::AThrowableWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WeaponSlotType = WeaponSlotType::Equipment;

	bGrenadeArmed = false;

	//PrimaryActorTick.bCanEverTick = true;
}

void AThrowableWeapon::StartFire()
{
	if (CurrentState == "Idle")
	{
		bGrenadeArmed = true;
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("FragGrenade Armed")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("FragGrenade Not Armed")));
	}
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("FragGrenade CurrentState = %s"), *CurrentState.ToString()));
}

void AThrowableWeapon::StopFire()
{
	TSubclassOf<ALastimProjectile> ProjClass;
	if (bGrenadeArmed && ProjectileClass.IsValidIndex(0))
	{
		FTransform SpawnTM(GetAdjustedAimRot(), GetAdjustedAimLoc()); //(ShootDir.Rotation(), JestVector);
		ProjClass = ProjectileClass[0];

		ALastimProjectile* Projectile = Cast<ALastimProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjClass, SpawnTM));
		if (Projectile)
		{
			Projectile->Instigator = Instigator;
			Projectile->SetOwner(this);

			if (MyPawn)
			{
				//FVector OutVel = MyPawn->GetVelocity();
				//Projectile->InitVelocity(OutVel);
			}

			UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
		}
		DestroyAfterUse();
	}
}

void AThrowableWeapon::DestroyAfterUse()
{
	ALastimCharacter* LOwner = Cast<ALastimCharacter>(Instigator);
	if (LOwner)
	{
		LOwner->RemoveFromInventory(this);
	}
	Destroy();
}