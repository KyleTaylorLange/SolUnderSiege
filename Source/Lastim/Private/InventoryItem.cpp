// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "InventoryItem.h"

//////////////////////////////////////////////////////////////////////////
// AInventoryItem

AInventoryItem::AInventoryItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("InventoryItem", "ItemName", "Unknown Item");
	DroppedPickupClass = ADroppedPickup::StaticClass();
	bReplicates = true;
}

void AInventoryItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

FString AInventoryItem::GetDisplayName() const
{
	return DisplayName.ToString();
}

void AInventoryItem::OnEnterInventory(ASolCharacter* NewOwner)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetOwningPawn(NewOwner);
	}
}

void AInventoryItem::OnLeaveInventory()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}
}

void AInventoryItem::SetOwningPawn(ASolCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		SetInstigator(NewOwner);
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}
}

ASolCharacter* AInventoryItem::GetOwningPawn()
{
	return MyPawn;
}

// We'll later make this actually return something for InventoryItem, but for now we just care about making it work for Weapon.
UMeshComponent* AInventoryItem::GetPickupMesh()
{
	return nullptr;
}

void AInventoryItem::Destroyed()
{
	if (MyPawn)
	{
		MyPawn->RemoveFromInventory(this);
		//GEngine->AddOnScreenDebugMessage(-1, 600.f, FColor::Red, FString::Printf(TEXT("~~~~%s: Item in inventory before destruction!~~~~"), *this->GetName()));
		UE_LOG(LogDamage, Warning, TEXT("DESTROYED(): %s still has owner!"), *this->GetName());

		////GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("%s: Item in %s inventory before destruction!"), *GetName(), *MyPawn->GetName()));
	}
	Super::Destroyed();
}

//////////////////////////////////////////////////////////////////////////
// Replication

void AInventoryItem::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInventoryItem, MyPawn);
}

void AInventoryItem::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}
