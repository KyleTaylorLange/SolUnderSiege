// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Bot_Domination.h"
#include "ObjectivePoint.h"
#include "DominationControlPoint.h"
#include "HUD_Domination.h"
#include "GameMode_Domination.h"

AGameMode_Domination::AGameMode_Domination(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Domination", "Domination", "Domination");

	BotAIControllerClass = ABot_Domination::StaticClass();
	HUDClass = AHUD_Domination::StaticClass();
	//GameStateClass = AGameState_Domination::StaticClass();

	/* Personal scores. */
	KillScore = 1;
	TeamkillScore = -1;
	DeathScore = 0;
	SuicideScore = -1;
	/* Team only scores from control points. */
	KillScoreForTeam = 0;
	TeamkillScoreForTeam = 0;
	DeathScoreForTeam = 0;
	SuicideScoreForTeam = 0;

	bDelayedStart = true;
}

void AGameMode_Domination::ProcessObjectivePoint(AObjectivePoint* InOP)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (InOP != nullptr)
		{
			/* Get spawn location from Objective Point's origin, then destroy it. */
			const FVector NewCPLocation = InOP->GetActorLocation();
			const FRotator NewCPRotation = InOP->GetActorRotation();
			InOP->Destroy();
			/* Spawn new Control Point. */
			FTransform SpawnTM(NewCPRotation, NewCPLocation);
			ADominationControlPoint* NewCP = Cast<ADominationControlPoint>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, ADominationControlPoint::StaticClass(), SpawnTM));
			if (NewCP)
			{
				UGameplayStatics::FinishSpawningActor(NewCP, SpawnTM);
			}
		}
	}
}


