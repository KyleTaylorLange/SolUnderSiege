// Copyright Kyle Taylor Lange

#pragma once
#include "Engine/LocalPlayer.h"
#include "SolLocalPlayer.generated.h"

/**
 * 
 */
//UCLASS(config=Engine) UCLASS(Within=Engine, config=Engine, transient)
UCLASS()
class SOL_API USolLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()

	// Overridden as a test.
	virtual FString GetNickname() const override;

	/* Sets player name to InName. */
	void SetPlayerName(FString InName);

	FLinearColor GetPrimaryColor() const;

	void SetPrimaryColor(FLinearColor NewColor);

	FLinearColor GetSecondaryColor() const;

	void SetSecondaryColor(FLinearColor NewColor);

protected:

	// The player's name.
	UPROPERTY(config)
	FString PlayerName;

	// The player's preferred primary color.
	UPROPERTY(config)
	FLinearColor PrimaryColor;

	// The player's preferred secondary color.
	UPROPERTY(config)
	FLinearColor SecondaryColor;
	
};
