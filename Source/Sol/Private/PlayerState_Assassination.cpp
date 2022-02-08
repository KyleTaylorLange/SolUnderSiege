// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "UnrealNetwork.h"
#include "PlayerState_Assassination.h"

APlayerState_Assassination::APlayerState_Assassination(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void APlayerState_Assassination::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerState_Assassination, AllowedKills);
}
