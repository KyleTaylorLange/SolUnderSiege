// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "UnrealNetwork.h"
#include "SolGameMode.h"
#include "SolGameState.h"

void ASolGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASolGameState, TeamCount);
	DOREPLIFETIME(ASolGameState, TeamArray);
	DOREPLIFETIME(ASolGameState, ScoreLimit);
	DOREPLIFETIME(ASolGameState, TimeLimit);
	DOREPLIFETIME(ASolGameState, RemainingTime);
	DOREPLIFETIME(ASolGameState, bTimerPaused);
	DOREPLIFETIME(ASolGameState, bForceRespawn);
	DOREPLIFETIME(ASolGameState, TeamColor);
}

void ASolGameState::GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const
{
	OutRankedMap.Empty();

	//first, we need to go over all the PlayerStates, grab their score, and rank them
	TMultiMap<int32, ASolPlayerState*> SortedMap;
	for (int32 i = 0; i < PlayerArray.Num(); ++i)
	{
		int32 Score = 0;
		ASolPlayerState* CurPlayerState = Cast<ASolPlayerState>(PlayerArray[i]);
		if (CurPlayerState && (CurPlayerState->GetTeamNum() == TeamIndex))
		{
			SortedMap.Add(FMath::TruncToInt(CurPlayerState->GetScore()), CurPlayerState);
		}
	}

	//sort by the keys
	SortedMap.KeySort(TGreater<int32>());

	//now, add them back to the ranked map
	OutRankedMap.Empty();

	int32 Rank = 0;
	for (TMultiMap<int32, ASolPlayerState*>::TIterator It(SortedMap); It; ++It)
	{
		OutRankedMap.Add(Rank++, It.Value());
	}
}

void ASolGameState::GetGameModes(TArray<UClass*>& GameModes) const
{
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* CurClass = (*It);
		if (CurClass->IsChildOf(ASolGameMode::StaticClass()) && !CurClass->HasAnyClassFlags(CLASS_Abstract))
		{
			GameModes.Add(CurClass);
		}
	}
}

void ASolGameState::GetMaps(TArray<FAssetData>& Maps, AGameMode* GameMode) const
{
	//TODO: Actually find maps.
}