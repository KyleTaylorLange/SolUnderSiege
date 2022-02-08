// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "UnrealNetwork.h"
#include "GameState_Domination.h"

void AGameState_Domination::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameState_Domination, ControlPoints);
}
