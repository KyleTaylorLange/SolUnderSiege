// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UnrealNetwork.h"
#include "LastimGameState.h"

void ALastimGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALastimGameState, TeamCount);
	DOREPLIFETIME(ALastimGameState, TeamArray);
	DOREPLIFETIME(ALastimGameState, ScoreLimit);
	DOREPLIFETIME(ALastimGameState, TimeLimit);
	DOREPLIFETIME(ALastimGameState, RemainingTime);
	DOREPLIFETIME(ALastimGameState, bTimerPaused);
	DOREPLIFETIME(ALastimGameState, bForceRespawn);
	DOREPLIFETIME(ALastimGameState, TeamColor);
}

void ALastimGameState::GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const
{
	OutRankedMap.Empty();

	//first, we need to go over all the PlayerStates, grab their score, and rank them
	TMultiMap<int32, ALastimPlayerState*> SortedMap;
	for (int32 i = 0; i < PlayerArray.Num(); ++i)
	{
		int32 Score = 0;
		ALastimPlayerState* CurPlayerState = Cast<ALastimPlayerState>(PlayerArray[i]);
		if (CurPlayerState && (CurPlayerState->GetTeamNum() == TeamIndex))
		{
			SortedMap.Add(FMath::TruncToInt(CurPlayerState->Score), CurPlayerState);
		}
	}

	//sort by the keys
	SortedMap.KeySort(TGreater<int32>());

	//now, add them back to the ranked map
	OutRankedMap.Empty();

	int32 Rank = 0;
	for (TMultiMap<int32, ALastimPlayerState*>::TIterator It(SortedMap); It; ++It)
	{
		OutRankedMap.Add(Rank++, It.Value());
	}
}