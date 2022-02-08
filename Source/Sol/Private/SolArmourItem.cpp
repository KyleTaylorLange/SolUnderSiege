// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolArmourItem.h"

ASolArmourItem::ASolArmourItem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Armour = 10.f;
	MaxArmour = 10.f;
	DamageAbsorptionPct = 0.5f;
}

float ASolArmourItem::GetArmour()
{
	return Armour;
}

void ASolArmourItem::SetArmour(float Armour)
{
	this->Armour = Armour;
}

void ASolArmourItem::ModifyDamageTaken(float& Damage, TSubclassOf<UDamageType> DamageType)
{
	if (Damage > 0.f)
	{
		float DamageAbsorbed = FMath::Min(Armour, Damage * DamageAbsorptionPct);
		SetArmour(Armour - DamageAbsorbed);
		Damage -= DamageAbsorbed;
	}
}
