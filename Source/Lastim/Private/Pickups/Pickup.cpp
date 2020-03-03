// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "InventoryItem.h"
#include "SolCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PickupSpawner.h"
#include "Pickup.h"

//////////////////////////////////////////////////////////////////////////
// APickup
//   Base class for item pickups.
//   Most functionality is in GenericPickup or SpecificPickup, but this class
//     contains some common functionality all pickups share.
//   TODO: Merge SpecificPickup into me since GenericPickup has been depreciated.
//   Future idea: merge my functionality into the Inventory class itself?
//                Why separate them when it totally depends on an inventory item?
//////////////////////////////////////////////////////////////////////////

APickup::APickup(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// We need some type of component as the root for this thing to spawn anywhere but the map origin.
	// TODO: Make an editor-only 3D mesh.
	TempShapeComp = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("TempSphereComp"));
	TempShapeComp->InitBoxExtent(FVector(25.0f, 25.0f, 25.0f));
	TempShapeComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TempShapeComp->SetCollisionResponseToAllChannels(ECR_Block); //(ECR_Ignore);
	TempShapeComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TempShapeComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	TempShapeComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	TempShapeComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	TempShapeComp->SetSimulatePhysics(true);
	//TempShapeComp->SetMassOverrideInKg(NAME_None, 20.f, true); GEngine error?
	RootComponent = TempShapeComp;
	//TempSphereComp->AttachToComponent(PickupMesh, FAttachmentTransformRules::KeepRelativeTransform);

	PickupMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("MeshComp"));

	PickupMesh->SetCollisionObjectType(ECC_PhysicsBody); //CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupMesh->AttachToComponent(TempShapeComp, FAttachmentTransformRules::KeepRelativeTransform);
	/*
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	*/
	//PickupMesh->SetSimulatePhysics(true);
	PickupMesh->SetSimulatePhysics(false);
	//PickupMesh->bChartDistanceFactor = true;
	PickupMesh->bReceivesDecals = false;
	PickupMesh->CastShadow = true;
	PickupMesh->SetHiddenInGame(false);
	//PickupMesh->SetRelativeRotation(FRotator::ZeroRotator);
	//PickupMesh->SetRelativeLocation(FVector::ZeroVector);
	//RootComponent = PickupMesh;
	//PickupMesh->AttachToComponent(TempShapeComp, FAttachmentTransformRules::KeepRelativeTransform);

	// Using a ProjectileMoveComp gives it the ability to fall and such without as many replication issues as just simulating physics would.
	UProjectileMovementComponent* ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = TempShapeComp; //PickupMesh;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = false; // true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Velocity = FVector(0.0f, 0.0f, -1.0f);
	ProjectileMovement->ProjectileGravityScale = 1.f;

	bReplicates = true;
	SetReplicatingMovement(true);
}

void APickup::CreatePickupMesh(class AInventoryItem* InItem)
{
	if (InItem)
	{
		USkeletalMeshComponent* TestSMC = Cast<USkeletalMeshComponent>(InItem->GetPickupMesh());
		if (TestSMC)
		{
			USkeletalMesh* TestSM = TestSMC->SkeletalMesh;
			if (TestSM)
			{
				PickupMesh->SetSkeletalMesh(TestSM);
				FTransform Dummy;
				FVector Bounds = PickupMesh->CalcBounds(Dummy).GetBox().GetExtent();
				FVector Center = PickupMesh->CalcBounds(Dummy).GetBox().GetCenter();
				PickupMesh->SetRelativeLocation(FVector(-Center.X, -Center.Y, -Center.Z));
				TempShapeComp->SetBoxExtent(Bounds);
				FString ItemName = InItem->GetDisplayName();
				//GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red, FString::Printf(TEXT("%s mesh bounds: %s, center: %s"), *ItemName, *Bounds.ToString(), *Center.ToString()));
			}
		}
		/*
		if (PickupMesh != NULL)
		{
			PickupMesh->DestroyComponent();
			PickupMesh = NULL;
		}
		// Not sure if all of this is necessary, but it works (there was an issue where the pickups would be deleted a second after spawning.)
		PickupMesh = ConstructObject<UMeshComponent>(InItem->GetPickupMesh()->GetClass(), this, NAME_None, RF_NoFlags, InItem->GetPickupMesh());
		PickupMesh->SetCollisionObjectType(ECC_PhysicsBody); //CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
		PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PickupMesh->SetCollisionResponseToAllChannels(ECR_Block); //(ECR_Ignore);
		PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		PickupMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		PickupMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		PickupMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
		PickupMesh->SetSimulatePhysics(true);
		//PickupMesh->bChartDistanceFactor = true;
		PickupMesh->bReceivesDecals = false;
		PickupMesh->CastShadow = true;
		PickupMesh->AttachParent = GetRootComponent(); //PickupMesh->AttachParent = NULL;
		PickupMesh->SetHiddenInGame(false);
		PickupMesh->RelativeRotation = FRotator::ZeroRotator;
		PickupMesh->RelativeLocation = FVector::ZeroVector;
		PickupMesh->RegisterComponent();
		//RootComponent = PickupMesh;
		//PickupMesh->DetachFromParent();
		////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s: CreatePickupMesh()"), *this->GetName()));
		*/
	}
}

AInventoryItem* APickup::GetHeldItem() const
{
	// Nothing here in the base class. This version should never be called!
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s: APickup::GetHeldItem() was called!"), *this->GetName()));
	return nullptr;
}

bool APickup::CanBePickedUp(class ASolCharacter* TestPawn) const
{
	return TestPawn && TestPawn->IsAlive();
}

void APickup::GivePickupTo(class ASolCharacter* Pawn)
{
	// Nothing here in the base class. This version should never be called!
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("%s: APickup::GivePickupTo() was called!"), *this->GetName()));
}

void APickup::PickupOnUse(class ASolCharacter* Pawn)
{
	if (Pawn && Pawn->IsAlive() && !IsPendingKill())
	{
		if (CanBePickedUp(Pawn))
		{
			GivePickupTo(Pawn);
			//PickedUpBy = Pawn;

			if (!IsPendingKill())
			{
				OnPickedUp();
			}
		}
	}
}

void APickup::OnPickedUp()
{
	Destroy();
}

void APickup::Destroyed()
{
	if (Spawner)
	{
		Spawner->OnPickupDestroyed(this);
	}
	Super::Destroyed();
}

float APickup::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	// Destroy pickup if it fell out of the map.
	//TSubclassOf<class DmgType_KillZ> KillZClass = Cast<DmgType_KillZ>(DamageEvent.DamageTypeClass);
	if (DamageEvent.DamageTypeClass == UDmgType_KillZ::StaticClass() )
	{
		Destroy();
	}
	return 0.0f;
}

void APickup::InitVelocity(FVector InVelocity)
{
	if (RootComponent)
	{
		//RootComponent->Velocity += InVelocity;
	}
}

//////////////////////////////////////////////////////////////////////////
// Replication