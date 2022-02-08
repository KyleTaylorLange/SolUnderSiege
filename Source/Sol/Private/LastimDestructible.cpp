// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "LastimDestructible.h"

//////////////////////////////////////////////////////////////////////////
// ALastimDestructible
// Base class of any environmental item (crate, door, lift) that can be damaged.

ALastimDestructible::ALastimDestructible(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetCanBeDamaged(false);
	Health = 100.f;
	Armour = 0.f;
}


float ALastimDestructible::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("LD Health: %f"), Health));

	if (Health <= 0.f)
	{
		return 0.f;
	}
	// Modify based on game rules.
	//ASolGameMode* const Game = GetWorld()->GetAuthGameMode<ASolGameMode>();
	//Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;
	Damage -= Armour;
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= ActualDamage;
		if (Health <= 0 && CanBeDamaged())
		{
			//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("LD Destroyed! Health: %f"), Health));
			Destroy(); //Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
			//MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
		}
		/**
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}
		**/
	}
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, FString::Printf(TEXT("LD Damage: %f"), ActualDamage));
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, FString::Printf(TEXT("LD Health: %f"), Health));
	return ActualDamage;
}

