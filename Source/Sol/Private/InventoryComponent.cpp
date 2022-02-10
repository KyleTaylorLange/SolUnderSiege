// Copyright Kyle Taylor Lange


#include "InventoryComponent.h"
#include "InventoryItem.h"
#include "UnrealNetwork.h"
#include "Sol.h"
// Temporarily until this is totally decoupled from SolCharacter
#include "SolCharacter.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicated(true);
	//PrimaryComponentTick.bCanEverTick = true;
	CurrentInventoryMass = 0.f;
	DefaultInventoryMassCapacity = 10.f;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInventoryComponent::AddToInventory(AInventoryItem* NewItem, AInventoryItem* OldItem)
{
	if (NewItem && GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		ASolCharacter* TEMP_OwningPawn = Cast<ASolCharacter>(GetOwner());
		if (CanHoldItem(NewItem) || CanSwapForItem(NewItem))
		{
			if (!CanHoldItem(NewItem))
			{
				OldItem = CanSwapForItem(NewItem);
				if (OldItem && TEMP_OwningPawn)
				{
					TEMP_OwningPawn->DropInventory(OldItem);
				}
			}
			ItemInventory.Add(NewItem);
			CurrentInventoryMass += NewItem->GetMassInInventory();
			NewItem->OnEnterInventory(TEMP_OwningPawn);
			// Check if NewItem is nullptr just in case it is immediately deleted by the above function.
			if (TEMP_OwningPawn && !TEMP_OwningPawn->GetEquippedItem() && NewItem && NewItem->CanBeEquipped())
			{
				TEMP_OwningPawn->EquipItem(NewItem);
			}
		}
	}
}

bool UInventoryComponent::RemoveFromInventory(AInventoryItem* ItemToRemove)
{
	if (ItemToRemove && GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		ASolCharacter* TEMP_OwningPawn = Cast<ASolCharacter>(GetOwner());
		ItemInventory.Remove(ItemToRemove);
		CurrentInventoryMass -= FMath::Min(ItemToRemove->GetMassInInventory(), CurrentInventoryMass);
		if (TEMP_OwningPawn->GetEquippedItem() == ItemToRemove)
		{
			TEMP_OwningPawn->EquipItem(nullptr);
		}
		ItemToRemove->OnLeaveInventory();
	}
	return true;
}

bool UInventoryComponent::CanHoldItem(AInventoryItem* Item) const
{
	return Item && CurrentInventoryMass + Item->GetMassInInventory() <= DefaultInventoryMassCapacity;
}

bool UInventoryComponent::ContainsItem(AInventoryItem* Item) const
{
	return ItemInventory.Contains(Item);
}

TArray<class AInventoryItem*> UInventoryComponent::GetInventory() const
{
	return ItemInventory;
}

AInventoryItem* UInventoryComponent::CanSwapForItem(AInventoryItem* Item) const
{
	// Can't swap if there's no equipped weapon to swap.
	ASolCharacter* TEMP_OwningPawn = Cast<ASolCharacter>(GetOwner());
	if (TEMP_OwningPawn && TEMP_OwningPawn->GetEquippedItem() && Item)
	{
		const float MassWithSwap = CurrentInventoryMass - TEMP_OwningPawn->GetEquippedItem()->GetMassInInventory() + Item->GetMassInInventory();
		if (MassWithSwap <= DefaultInventoryMassCapacity)
		{
			return TEMP_OwningPawn->GetEquippedItem();
		}
	}
	return nullptr;
}

int32 UInventoryComponent::GetInventoryCount() const
{
	return ItemInventory.Num();
}

float UInventoryComponent::GetCurrentInventoryMass() const
{
	return CurrentInventoryMass;
}

float UInventoryComponent::GetMaxInventoryMass() const
{
	return DefaultInventoryMassCapacity;
}

void UInventoryComponent::SetMaxInventoryMass(float MaxMass)
{
	DefaultInventoryMassCapacity = MaxMass;
}

void UInventoryComponent::DestroyInventory()
{
	ASolCharacter* TEMP_OwningPawn = Cast<ASolCharacter>(GetOwner());
	for (int32 i = 0; i < ItemInventory.Num(); i++)
	{
		UE_LOG(LogDamage, Warning, TEXT("%s: Deleting Item %s."), *GetName(), *ItemInventory[i]->GetName());
		if (ItemInventory[i]->GetOwningPawn() != TEMP_OwningPawn && ItemInventory[i]->GetOwningPawn() != nullptr)
		{
			UE_LOG(LogDamage, Warning, TEXT("%s: Item %s belonged to another pawn!"), *GetName(), *ItemInventory[i]->GetName());
		}
		ItemInventory[i]->SetOwningPawn(nullptr);
		ItemInventory[i]->Destroy();
	}
}

void UInventoryComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	DestroyInventory();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner: weapon change requests are locally instigated, other clients don't need it
	DOREPLIFETIME(UInventoryComponent, ItemInventory);
	//DOREPLIFETIME_CONDITION(UInventoryComponent, ItemInventory, COND_OwnerOnly);
	DOREPLIFETIME(UInventoryComponent, CurrentInventoryMass);
	DOREPLIFETIME(UInventoryComponent, DefaultInventoryMassCapacity);
}
