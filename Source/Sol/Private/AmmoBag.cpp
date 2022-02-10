// Copyright Kyle Taylor Lange

#include "AmmoBag.h"
#include "Sol.h"
#include "SolCharacter.h"
#include "InventoryComponent.h"

FString AAmmoBag::GetDisplayName() const
{
	// This assumes the items are all the same.
	if (ContainedItems.Num() > 0)
	{
		return FString::Printf(TEXT("AB: %s x%d"), *ContainedItems[0]->GetDisplayName(), ContainedItems.Num());
	}
	else
	{
		return FString::Printf(TEXT("Empty Ammo Bag")); //return Super::GetDisplayName();
	}
}

void AAmmoBag::OnEnterInventory(ASolCharacter* NewOwner)
{
	Super::OnEnterInventory(NewOwner);
	if (ContainedItems.Num() > 0)
	{
		for (int32 i = 0; i < ContainedItems.Num(); i++)
		{
			if (ContainedItems[i] != nullptr)
			{
				NewOwner->GetInventoryComponent()->AddToInventory(ContainedItems[i]);
			}
		}
		ContainedItems.Empty();
		if (MyPawn)
		{
			MyPawn->GetInventoryComponent()->RemoveFromInventory(this);
		}
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("AB %s should be destroyed!"), *GetDisplayName()));
		Destroy();
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("AB %s must be destroyed!"), *GetDisplayName()));
	}
}
