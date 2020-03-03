// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterface.h"
#include "SolLocalPlayer.h"

USolLocalPlayer::USolLocalPlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
	PlayerName = FString("SolPlayer");
}

FString USolLocalPlayer::GetNickname() const
{
	// Use Steam name if Steam works.
	// TODO: Make system where player can either use Steam name (which dynamically updates)
	//       or a custom name if entered in the options screen.
	FName Steam = FName(TEXT("Steam"));
	IOnlineSubsystem* const SteamOSS = IOnlineSubsystem::Get(Steam);
	if (SteamOSS)
	{
		// Old code gets Steam name (Dakatsu) or the PC name (Kyle-PC-0F31...).
		UWorld* World = GetWorld();
		if (World != NULL)
		{
			IOnlineIdentityPtr OnlineIdentityInt = Online::GetIdentityInterface(World);
			if (OnlineIdentityInt.IsValid())
			{
				auto UniqueId = GetPreferredUniqueNetId();
				if (UniqueId.IsValid())
				{
					return OnlineIdentityInt->GetPlayerNickname(*UniqueId);
				}
			}
		}
	}

	return PlayerName;
}

void USolLocalPlayer::SetPlayerName(FString InName)
{
	if (InName == FString(""))
	{
		if (Super::GetNickname() == FString(""))
		{
			PlayerName = FString("SolPlayer");
		}
		else
		{
			PlayerName = Super::GetNickname();
		}
	}
	else
	{
		PlayerName = InName;
	}
	SaveConfig();
}


