// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "UnrealNetwork.h"
#include "InventoryItem.h"
#include "SolCharacter.h"
#include "DmgType_KillZ.h"
#include "InventoryItem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PickupSpawner.h"
#include "Pickup.h"
#include "Interaction_PickUpItem.h"
#include "Interaction_SwapForItem.h"

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
	TempShapeComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TempShapeComp->SetCollisionResponseToAllChannels(ECR_Block); //(ECR_Ignore);
	TempShapeComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TempShapeComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	TempShapeComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	TempShapeComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	TempShapeComp->SetSimulatePhysics(true);
	UBoxComponent* BoxRootComp = Cast<UBoxComponent>(TempShapeComp);
	if (BoxRootComp)
	{
		BoxRootComp->InitBoxExtent(FVector(12.5f, 12.5f, 12.5f));
	}
	//TempShapeComp->SetMassOverrideInKg(NAME_None, 20.f, true); GEngine error?
	RootComponent = TempShapeComp;

	// Pickup mesh is made dynamically when an object is assigned.
	PickupMesh = nullptr;

	// Using a ProjectileMoveComp gives it the ability to fall and such without as many replication issues as just simulating physics would.
	UProjectileMovementComponent* ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = TempShapeComp; //PickupMesh;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = false; // true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Velocity = FVector(0.0f, 0.0f, -1.0f);
	ProjectileMovement->ProjectileGravityScale = 1.f;

	UInteractableComponent* InteractComp = ObjectInitializer.CreateDefaultSubobject<UInteractableComponent>(this, TEXT("InteractableComponent"));
	InteractComp->CanInteractWithDelegate.BindUObject(this, &APickup::CanInteractWith);
	InteractComp->OnStartUseByDelegate.BindUObject(this, &APickup::OnStartUseBy);
	InteractComp->OnStopUseByDelegate.BindUObject(this, &APickup::OnStopUseBy);
	InteractComp->InteractionEvents.Empty();
	InteractComp->InteractionEvents.Add(UInteraction_PickUpItem::StaticClass());
	InteractComp->InteractionEvents.Add(UInteraction_SwapForItem::StaticClass());

	HeldItem = nullptr;

	bReplicates = true;
	SetReplicatingMovement(true);
}

void APickup::CreatePickupMesh(class AInventoryItem* InItem)
{
	if (InItem)
	{
		UMeshComponent* InItemMesh = InItem->GetPickupMesh();
		USkeletalMeshComponent* SkelMeshComp = Cast<USkeletalMeshComponent>(InItemMesh);
		UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(InItemMesh);
		if (SkelMeshComp)
		{
			USkeletalMesh* SkelMesh = SkelMeshComp->SkeletalMesh;
			if (SkelMesh)
			{
				if (PickupMesh)
				{
					PickupMesh->DestroyComponent();
					PickupMesh = nullptr;
				}
				USceneComponent* NewComponent = NewObject<USceneComponent>(this, USkeletalMeshComponent::StaticClass());
				USkeletalMeshComponent* NewSkelMeshComp = Cast<USkeletalMeshComponent>(NewComponent);
				NewSkelMeshComp->SetSkeletalMesh(SkelMesh);
				NewSkelMeshComp->SetRelativeScale3D(SkelMeshComp->GetRelativeScale3D());
				PickupMesh = NewSkelMeshComp;
				PickupMesh->RegisterComponent();
				//GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red, FString::Printf(TEXT("%s mesh bounds: %s, center: %s"), *ItemName, *Bounds.ToString(), *Center.ToString()));
			}
		}
		else if (StaticMeshComp)
		{
			UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh();
			if (StaticMesh)
			{
				if (PickupMesh)
				{
					PickupMesh->DestroyComponent();
					PickupMesh = nullptr;
				}
				USceneComponent* NewComponent = NewObject<USceneComponent>(this, UStaticMeshComponent::StaticClass());
				UStaticMeshComponent* NewStaticMeshComp = Cast<UStaticMeshComponent>(NewComponent);
				NewStaticMeshComp->SetStaticMesh(StaticMesh);
				NewStaticMeshComp->OverrideMaterials = StaticMeshComp->OverrideMaterials;
				NewStaticMeshComp->SetRelativeScale3D(StaticMeshComp->GetRelativeScale3D());
				PickupMesh = NewStaticMeshComp;
				PickupMesh->RegisterComponent();
				//GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Red, FString::Printf(TEXT("%s mesh bounds: %s, center: %s"), *ItemName, *Bounds.ToString(), *Center.ToString()));
			}
		}

		if (PickupMesh)
		{
			PickupMesh->SetCollisionObjectType(ECC_PhysicsBody); //CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
			PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PickupMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			PickupMesh->AttachToComponent(TempShapeComp, FAttachmentTransformRules::KeepRelativeTransform);
			PickupMesh->SetSimulatePhysics(false);
			//PickupMesh->bChartDistanceFactor = true;
			PickupMesh->bReceivesDecals = false;
			PickupMesh->CastShadow = true;
			PickupMesh->SetHiddenInGame(false);

			FTransform Dummy;
			FVector Bounds = PickupMesh->CalcBounds(Dummy).GetBox().GetExtent();
			FVector Center = PickupMesh->CalcBounds(Dummy).GetBox().GetCenter();
			FString ItemName = InItem->GetDisplayName();
			PickupMesh->SetRelativeLocation(FVector(-Center.X, -Center.Y, -Center.Z));

			UBoxComponent* BoxRootComp = Cast<UBoxComponent>(TempShapeComp);
			if (BoxRootComp)
			{
				BoxRootComp->SetBoxExtent(Bounds);
			}
		}

		/*
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
	return HeldItem;
}

void APickup::SetHeldItem(AInventoryItem* InItem)
{
	HeldItem = InItem;
	if (HeldItem)
	{
		CreatePickupMesh(HeldItem);
	}
}

bool APickup::CanBePickedUp(class ASolCharacter* TestPawn) const
{
	return TestPawn && TestPawn->IsAlive();
}

void APickup::GivePickupTo(class ASolCharacter* Pawn)
{
	if (HeldItem)
	{
		Pawn->AddToInventory(HeldItem);
		// Set item to null so it doesn't get destroyed when pickup is destroyed.
		HeldItem = nullptr;
	}
}

bool APickup::CanInteractWith(UInteractableComponent* Component, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction)
{
	ASolCharacter* UsingCharacter = Cast<ASolCharacter>(Interactor);
	if (UsingCharacter && CanBePickedUp(UsingCharacter))
	{
		if (Interaction == UInteraction_PickUpItem::StaticClass())
		{
			return UsingCharacter->CanHoldItem(GetHeldItem());
		}
		if (Interaction == UInteraction_SwapForItem::StaticClass())
		{
			return UsingCharacter->CanSwapForItem(GetHeldItem()) != nullptr;
		}
	}
	return false;
}

void APickup::OnStartUseBy(UInteractableComponent* Component, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction)
{
	if (ASolCharacter* SChar = Cast<ASolCharacter>(Interactor))
	{
		PickupOnUse(SChar);
	}
}

void APickup::OnStopUseBy(UInteractableComponent* Component, AActor* Interactor, TSubclassOf<UInteractionEvent> Interaction)
{
	// Nothing for now.
}

void APickup::PickupOnUse(class ASolCharacter* Pawn)
{
	if (Pawn && Pawn->IsAlive() && !IsPendingKill())
	{
		if (CanBePickedUp(Pawn))
		{
			GivePickupTo(Pawn);

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
	if (HeldItem)
	{
		HeldItem->Destroy();
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

void APickup::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickup, HeldItem);
}

void APickup::OnRep_HeldItem()
{
	CreatePickupMesh(HeldItem);
}
