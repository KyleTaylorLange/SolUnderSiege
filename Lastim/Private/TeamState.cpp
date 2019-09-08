// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "TeamState.h"

void ATeamState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATeamState, TeamName);
	DOREPLIFETIME(ATeamState, Score);
	DOREPLIFETIME(ATeamState, Kills);
	DOREPLIFETIME(ATeamState, Deaths);
	DOREPLIFETIME(ATeamState, Suicides);
	DOREPLIFETIME(ATeamState, Teamkills);
	DOREPLIFETIME(ATeamState, TeamIndex);
	DOREPLIFETIME(ATeamState, TeamColor);
}

FText ATeamState::GetTeamName() const
{
	return TeamName;
}

void ATeamState::SetTeamName(FText InName)
{
	TeamName = InName;
}

float ATeamState::GetScore() const
{
	return Score;
}

void ATeamState::AddScore(int32 InScore)
{
	Score += InScore;
}

int32 ATeamState::GetKills() const
{
	return Kills;
}

void ATeamState::AddKill()
{
	Kills++;
}

int32 ATeamState::GetDeaths() const
{
	return Deaths;
}

void ATeamState::AddDeath()
{
	Deaths++;
}

int32 ATeamState::GetSuicides() const
{
	return Suicides;
}

void ATeamState::AddSuicide()
{
	Deaths++;
	Suicides++;
}

int32 ATeamState::GetTeamkills() const
{
	return Teamkills;
}

void ATeamState::AddTeamkill()
{
	Teamkills++;
}

int32 ATeamState::GetTeamIndex() const
{
	return TeamIndex;
}

void ATeamState::SetTeamIndex(int32 NewIndex)
{
	TeamIndex = NewIndex;
}

FLinearColor ATeamState::GetTeamColor() const
{
	return TeamColor;
}

void ATeamState::SetTeamColor(FLinearColor NewColor)
{
	TeamColor = NewColor;
}


