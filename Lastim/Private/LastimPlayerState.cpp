// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimCharacter.h"
#include "LastimGameMode.h"
#include "LastimGameState.h"
#include "TeamState.h"
#include "LastimPlayerState.h"

ALastimPlayerState::ALastimPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Kills = 0;
	Deaths = 0;
	Suicides = 0;
	Teamkills = 0;

	PrimaryActorTick.bCanEverTick = true;

	TeamNumber = 255;

	PrimaryColor = FLinearColor(0.25f, 0.25f, 0.35f);
	SecondaryColor = FLinearColor(0.75f, 0.75f, 0.75f);
}

void ALastimPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void ALastimPlayerState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Tick down respawn time. */
	if (RespawnTime > 0.0f)
	{
		RespawnTime -= DeltaTime;
	}
}

int32 ALastimPlayerState::GetKills() const
{
	return Kills;
}

int32 ALastimPlayerState::GetDeaths() const
{
	return Deaths;
}

int32 ALastimPlayerState::GetSuicides() const
{
	return Suicides;
}

float ALastimPlayerState::GetScore() const
{
	return Score;
}

int32 ALastimPlayerState::GetTeamNum() const
{
	int32 TeamNum = 255;
	if (GetTeam())
	{
		TeamNum = GetTeam()->GetTeamIndex();
	}
	return TeamNum;
}

ATeamState* ALastimPlayerState::GetTeam() const
{
	return Team;
}

FLinearColor ALastimPlayerState::GetPrimaryColor() const
{
	return PrimaryColor;
}

FLinearColor ALastimPlayerState::GetSecondaryColor() const
{
	return SecondaryColor;
}



void ALastimPlayerState::AddKill(ALastimPlayerState* Victim)
{
	Kills++;
}

void ALastimPlayerState::AddDeath(ALastimPlayerState* KilledBy)
{
	Deaths++;
	//TEST
	ALastimGameMode* LGM = Cast<ALastimGameMode>(GetWorld()->GetAuthGameMode());
	if (LGM)
	{
		RespawnTime = LGM->GetRespawnTime(this);
	}
}

void ALastimPlayerState::AddSuicide()
{
	Deaths++;
	Suicides++;
	//TEST
	ALastimGameMode* LGM = Cast<ALastimGameMode>(GetWorld()->GetAuthGameMode());
	if (LGM)
	{
		RespawnTime = LGM->GetRespawnTime(this);
	}
}

void ALastimPlayerState::AddTeamkill(ALastimPlayerState* Victim)
{
	Teamkills++;
}

void ALastimPlayerState::AddScore(int32 InScore)
{
	Score += InScore;
}

void ALastimPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
	UpdateTeamColors();
}

void ALastimPlayerState::SetTeam(ATeamState* NewTeam)
{
	Team = NewTeam;
	if (Team)
	{
		TeamNumber = Team->GetTeamIndex();
	}
	UpdateTeamColors();
}

void ALastimPlayerState::SetPrimaryColor(FLinearColor NewColor)
{
	PrimaryColor = NewColor;
}

void ALastimPlayerState::SetSecondaryColor(FLinearColor NewColor)
{
	SecondaryColor = NewColor;
}

void ALastimPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		ALastimCharacter* LastimCharacter = Cast<ALastimCharacter>(OwnerController->GetCharacter());
		if (LastimCharacter != NULL)
		{
			LastimCharacter->UpdateAllMaterials();
		}
	}
}

void ALastimPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALastimPlayerState, Kills);
	DOREPLIFETIME(ALastimPlayerState, Deaths);
	DOREPLIFETIME(ALastimPlayerState, Suicides);
	DOREPLIFETIME(ALastimPlayerState, Teamkills);
	DOREPLIFETIME(ALastimPlayerState, TeamNumber);
	DOREPLIFETIME(ALastimPlayerState, Team);
	DOREPLIFETIME(ALastimPlayerState, PrimaryColor);
	DOREPLIFETIME(ALastimPlayerState, SecondaryColor);
	DOREPLIFETIME(ALastimPlayerState, RespawnTime);
}
