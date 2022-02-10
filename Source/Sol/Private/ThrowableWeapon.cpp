// Copyright Kyle Taylor Lange

#include "ThrowableWeapon.h"
#include "Sol.h"
#include "SolCharacter.h"
#include "InventoryComponent.h"

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
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("FragGrenade Armed")));
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("FragGrenade Not Armed")));
	}
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("FragGrenade CurrentState = %s"), *CurrentState.ToString()));
}

void AThrowableWeapon::StopFire()
{
	TSubclassOf<AProjectile> ProjClass;
	if (bGrenadeArmed && ProjectileClass.IsValidIndex(0))
	{
		FTransform SpawnTM(GetAdjustedAimRot(), GetAdjustedAimLoc()); //(ShootDir.Rotation(), JestVector);
		ProjClass = ProjectileClass[0];

		AProjectile* Projectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ProjClass, SpawnTM));
		if (Projectile)
		{
			Projectile->SetInstigator(GetInstigator());
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
	if (ASolCharacter* SolOwner = Cast<ASolCharacter>(GetInstigator()))
	{
		SolOwner->GetInventoryComponent()->RemoveFromInventory(this);
	}
	Destroy();
}