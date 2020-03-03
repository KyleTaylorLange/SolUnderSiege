// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "SolCharacter.h"
#include "SolGameMode.h"
#include "SolGameState.h"
#include "TeamState.h"
#include "SolPlayerState.h"

ASolPlayerState::ASolPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Kills = 0;
	Deaths = 0;
	Suicides = 0;
	Teamkills = 0;

	PrimaryActorTick.bCanEverTick = true;

	TeamNumber = -1;

	PrimaryColor = FLinearColor(0.25f, 0.25f, 0.35f);
	SecondaryColor = FLinearColor(0.75f, 0.75f, 0.75f);
}

void ASolPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void ASolPlayerState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/* Tick down respawn time. */
	if (RespawnTime > 0.0f)
	{
		RespawnTime -= DeltaTime;
	}
}

int32 ASolPlayerState::GetKills() const
{
	return Kills;
}

int32 ASolPlayerState::GetDeaths() const
{
	return Deaths;
}

int32 ASolPlayerState::GetSuicides() const
{
	return Suicides;
}

float ASolPlayerState::GetScore() const
{
	return Score;
}

int32 ASolPlayerState::GetTeamNum() const
{
	int32 TeamNum = -1;
	if (GetTeam())
	{
		TeamNum = GetTeam()->GetTeamIndex();
	}
	return TeamNum;
}

ATeamState* ASolPlayerState::GetTeam() const
{
	return Team;
}

FLinearColor ASolPlayerState::GetPrimaryColor() const
{
	return PrimaryColor;
}

FLinearColor ASolPlayerState::GetSecondaryColor() const
{
	return SecondaryColor;
}



void ASolPlayerState::AddKill(ASolPlayerState* Victim)
{
	Kills++;
}

void ASolPlayerState::AddDeath(ASolPlayerState* KilledBy)
{
	Deaths++;
	//TEST
	ASolGameMode* LGM = Cast<ASolGameMode>(GetWorld()->GetAuthGameMode());
	if (LGM)
	{
		RespawnTime = LGM->GetRespawnTime(this);
	}
}

void ASolPlayerState::AddSuicide()
{
	Deaths++;
	Suicides++;
	//TEST
	ASolGameMode* LGM = Cast<ASolGameMode>(GetWorld()->GetAuthGameMode());
	if (LGM)
	{
		RespawnTime = LGM->GetRespawnTime(this);
	}
}

void ASolPlayerState::AddTeamkill(ASolPlayerState* Victim)
{
	Teamkills++;
}

void ASolPlayerState::AddScore(int32 InScore)
{
	Score += InScore;
}

void ASolPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;
	UpdateTeamColors();
}

void ASolPlayerState::SetTeam(ATeamState* NewTeam)
{
	Team = NewTeam;
	if (Team)
	{
		TeamNumber = Team->GetTeamIndex();
	}
	UpdateTeamColors();
}

void ASolPlayerState::SetPrimaryColor(FLinearColor NewColor)
{
	PrimaryColor = NewColor;
}

void ASolPlayerState::SetSecondaryColor(FLinearColor NewColor)
{
	SecondaryColor = NewColor;
}

void ASolPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		ASolCharacter* SolCharacter = Cast<ASolCharacter>(OwnerController->GetCharacter());
		if (SolCharacter != NULL)
		{
			SolCharacter->UpdateAllMaterials();
		}
	}
}

void ASolPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASolPlayerState, Kills);
	DOREPLIFETIME(ASolPlayerState, Deaths);
	DOREPLIFETIME(ASolPlayerState, Suicides);
	DOREPLIFETIME(ASolPlayerState, Teamkills);
	DOREPLIFETIME(ASolPlayerState, TeamNumber);
	DOREPLIFETIME(ASolPlayerState, Team);
	DOREPLIFETIME(ASolPlayerState, PrimaryColor);
	DOREPLIFETIME(ASolPlayerState, SecondaryColor);
	DOREPLIFETIME(ASolPlayerState, RespawnTime);
}
