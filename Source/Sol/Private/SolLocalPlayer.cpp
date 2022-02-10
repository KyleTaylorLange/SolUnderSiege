// Copyright Kyle Taylor Lange

#include "SolLocalPlayer.h"
#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterface.h"
#include "SolTypes.h"

USolLocalPlayer::USolLocalPlayer(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
	PlayerName = FString("SolPlayer");
	PrimaryColor = SOL_COLOR_SILVER;
	SecondaryColor = SOL_COLOR_SILVER;
}

FString USolLocalPlayer::GetNickname() const
{
	// TODO: Make system where player can either use Steam name (which dynamically updates)
	//       or a custom name if entered in the options screen.
	FName Steam = FName(TEXT("Steam"));
	if (IOnlineSubsystem* const SteamOSS = IOnlineSubsystem::Get(Steam))
	{
		// Old code gets Steam name (Dakatsu) or the PC name (Kyle-PC-0F31...).
		if (UWorld* World = GetWorld())
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

FLinearColor USolLocalPlayer::GetPrimaryColor() const
{
	return PrimaryColor;
}

void USolLocalPlayer::SetPrimaryColor(FLinearColor NewColor)
{
	PrimaryColor = NewColor;
	SaveConfig();
}

FLinearColor USolLocalPlayer::GetSecondaryColor() const
{
	return SecondaryColor;
}

void USolLocalPlayer::SetSecondaryColor(FLinearColor NewColor)
{
	SecondaryColor = NewColor;
	SaveConfig();
}
