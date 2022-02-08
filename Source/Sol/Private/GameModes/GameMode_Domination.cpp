// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "Bot_Domination.h"
#include "ObjectivePoint.h"
#include "DominationControlPoint.h"
#include "HUD_Domination.h"
#include "TeamState.h"
#include "GameState_Domination.h"
#include "GameMode_Domination.h"

AGameMode_Domination::AGameMode_Domination(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("GameMode_Domination", "Domination", "Domination");

	BotAIControllerClass = ABot_Domination::StaticClass();
	HUDClass = AHUD_Domination::StaticClass();
	GameStateClass = AGameState_Domination::StaticClass();

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

void AGameMode_Domination::OnScoreCP(ADominationControlPoint* InCP)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// Score point for owning team if we're in progress.
		if (GetMatchState() == "InProgress" && InCP && InCP->GetOwningTeam())
		{
			InCP->GetOwningTeam()->AddScore(1);
			if (InCP->GetOwningPlayer())
			{
				InCP->GetOwningPlayer()->AddScore(1);
			}
			CheckForMatchWinner();
		}
	}
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
				NewCP->OnScoreCPDelegate.BindUObject(this, &AGameMode_Domination::OnScoreCP);
				AGameState_Domination* DomGS = Cast<AGameState_Domination>(GameState);
				if (DomGS)
				{
					DomGS->ControlPoints.Add(NewCP);
				}
			}
		}
	}
}

AActor* AGameMode_Domination::ChoosePlayerStart_Implementation(AController* Player)
{
	// Attempt to find control point we can spawn on.
	UWorld* World = GetWorld();
	UClass* PawnClass = GetDefaultPawnClassForController(Player);
	APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	AGameState_Domination* DomGS = Cast<AGameState_Domination>(GameState);
	TArray<ADominationControlPoint*> AlliedControlPoints;

	// Find the player's team.
	ASolPlayerState* SolPlayer = Cast<ASolPlayerState>(Player->PlayerState);
	ATeamState* DesiredTeam = nullptr;
	if (SolPlayer)
	{
		DesiredTeam = SolPlayer->GetTeam();
	}

	// Find a control point to spawn on.
	if (DomGS != nullptr && DesiredTeam != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Looking at starts from %d options."), DomGS->ControlPoints.Num()));
		for (ADominationControlPoint* ControlPoint : DomGS->ControlPoints)
		{
			if (ControlPoint->GetOwningTeam() == DesiredTeam)
			{
				FVector ActorLocation = ControlPoint->GetActorLocation();
				const FRotator ActorRotation = ControlPoint->GetActorRotation();
				//if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
				{
					AlliedControlPoints.Add(ControlPoint);
				}
			}
		}
		// Pick a control point to spawn on if we found one.
		if (AlliedControlPoints.Num() > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Picking Control Point from %d options."), AlliedControlPoints.Num()));
			return AlliedControlPoints[FMath::RandRange(0, AlliedControlPoints.Num() - 1)];
		}
	}

	// If no control points, just spawn at a player start.
	return Super::ChoosePlayerStart_Implementation(Player);
}
