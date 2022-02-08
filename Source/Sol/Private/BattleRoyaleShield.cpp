// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolCharacter.h"
#include "BattleRoyaleShield.h"

ABattleRoyaleShield::ABattleRoyaleShield(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("BattleRoyaleShield", "BattleRoyaleShield", "Battle Royale Shield");

	bCanBeEquipped = false;
	Armour = 150.f;
	DrainPerTick = 5.f;
	TimeBetweenDrainTicks = 3.f;
	bModifiesDamage = false;
}

void ABattleRoyaleShield::ModifyDamageTaken(float& Damage, TSubclassOf<UDamageType> DamageType)
{
	if (bModifiesDamage)
	{
		Super::ModifyDamageTaken(Damage, DamageType);
	}
}

void ABattleRoyaleShield::OnEnterInventory(ASolCharacter* NewOwner)
{
	Super::OnEnterInventory(NewOwner);
	GetWorldTimerManager().SetTimer(TimerHandle_Drain, this, &ABattleRoyaleShield::OnDrain, CalculateNextDrainTime());
}

void ABattleRoyaleShield::OnLeaveInventory()
{
	Super::OnLeaveInventory();
	GetWorldTimerManager().ClearTimer(TimerHandle_Drain);
}

void ABattleRoyaleShield::OnDrain()
{
	float DrainAsArmour = FMath::Min(DrainPerTick, Armour);
	Armour -= DrainAsArmour;
	float DrainAsDamage = FMath::Max(0.0f, DrainPerTick - DrainAsArmour);
	if (GetOwningPawn() && GetOwningPawn()->IsAlive())
	{
		if (DrainAsDamage > 0)
		{
			FDamageEvent DummyDamageEvent;
			// TODO: Create damage type that bypasses armour. This should directly damage player.
			GetOwningPawn()->TakeDamage(DrainAsDamage, DummyDamageEvent, nullptr, this);
		}
		GetWorldTimerManager().SetTimer(TimerHandle_Drain, this, &ABattleRoyaleShield::OnDrain, CalculateNextDrainTime());
	}
}

float ABattleRoyaleShield::CalculateNextDrainTime()
{
	return TimeBetweenDrainTicks;
}

FString ABattleRoyaleShield::GetDisplayName() const
{
	if (Armour > 0)
	{
		return Super::GetDisplayName() + FString(" (") + FString::FromInt(Armour) + FString(")");
	}
	return Super::GetDisplayName();
}
