// Fill out your copyright notice in the Description page of Project Settings.

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "Weapon.h"

//////////////////////////////////////////////////////////////////////////
// AWeapon

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("Weapon", "ItemName", "Unknown Weapon");
	
	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	//Mesh1P->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	RootComponent = Mesh1P;

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	//Mesh3P->bChartDistanceFactor = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->CastShadow = true;
	Mesh3P->SetupAttachment(Mesh1P);

	WeaponSlotType = WeaponSlotType::Equipment;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;
}

void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	/** Create material instances so we can manipulate them later (for ammo indicators and such). **/
	for (int32 iMat = 0; iMat < Mesh1P->GetNumMaterials(); iMat++) //(int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(Mesh1P->CreateAndSetMaterialInstanceDynamic(iMat));
	}
}

FRotator AWeapon::GetAdjustedAimRot() const
{
	FRotator FinalRot;
	if (MyPawn)
	{
		FinalRot= MyPawn->GetWeaponAimRot();
	}
	return FinalRot;
}

FVector AWeapon::GetAdjustedAimLoc() const
{
	FVector FinalLoc;
	if (MyPawn)
	{
		FinalLoc = MyPawn->GetWeaponAimLoc() + (MyPawn->GetWeaponAimRot().Vector() * 20.f);
	}
	return FinalLoc;
}

FVector AWeapon::GetAimPoint() const
{
	// Just do a simple straight line trace for now, even for projectile weapons.
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), true, GetInstigator());
	FHitResult Hit(ForceInit);
	FVector EndPoint = GetAdjustedAimLoc() + GetAdjustedAimRot().Vector() * 10000.f;
	GetWorld()->LineTraceSingleByChannel(Hit, GetAdjustedAimLoc(), EndPoint, COLLISION_PROJECTILE, TraceParams);
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("HitInfo: %s"), *Hit.ToString()));
	if (Hit.bBlockingHit)
	{
		return Hit.ImpactPoint;
	}
	return EndPoint;
	// Very basic implementation for most projectiles.
	/**if (ProjectileClass.Num() > 0 && ProjectileClass[0] != nullptr)
	{
		FPredictProjectilePathParams Params;
		Params.LaunchVelocity = GetAdjustedAimRot().Vector() * ProjectileClass[0].GetDefaultObject()->GetVelocity().X;
		Params.StartLocation = GetAdjustedAimLoc();
		Params.OverrideGravityZ = -0.01f;
		Params.MaxSimTime = 5.f;
		Params.bTraceComplex = true;
		Params.bTraceWithCollision = true;
		Params.ProjectileRadius = 1.f;

		FPredictProjectilePathResult Result;
		if (!UGameplayStatics::PredictProjectilePath(this, Params, Result))
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("LastTrace is %s"), *Result.LastTraceDestination.Location.ToString()));
			return Result.LastTraceDestination.Location;
		}
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("ImpactPoint is %s"), *Result.HitResult.ImpactPoint.ToString()));
		return Result.HitResult.ImpactPoint;
	}
	return FVector::ZeroVector;
	*/
}

float AWeapon::GetAIRating()
{
	return BaseAIRating;
}
