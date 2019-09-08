// Copyright Kyle Taylor Lange

#pragma once
#include "Engine/LocalPlayer.h"
#include "LastimLocalPlayer.generated.h"

/**
 * 
 */
//UCLASS(config=Engine) UCLASS(Within=Engine, config=Engine, transient)
UCLASS()
class LASTIM_API ULastimLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()

public:

	// Overridden as a test.
	virtual FString GetNickname() const override;

	/* Sets player name to InName. */
	void SetPlayerName(FString InName);

protected:

	// The player's name. Temporary.
	UPROPERTY(config)
	FString PlayerName;
	
};
