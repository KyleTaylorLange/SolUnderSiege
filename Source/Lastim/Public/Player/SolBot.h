// Copyright Kyle Taylor Lange

#pragma once
#include "Player/SolAIController.h"
#include "SolBot.generated.h"

/**
 * Profile of a given bot's personality.
 */
USTRUCT(BlueprintType)
struct FBotProfile
{
	GENERATED_USTRUCT_BODY()

	/* Bot's name. */
	UPROPERTY()
	FString BotName;

	/* Bot's gender: false for male, true for female. */
	UPROPERTY()
	bool bIsFemale;

	/* Accuracy rating. */
	UPROPERTY()
	float Accuracy;

	/* Alertness rating. */
	UPROPERTY()
	float Alertness;

	UPROPERTY()
	FLinearColor PrimaryColor;

	UPROPERTY()
	FLinearColor SecondaryColor;

	FBotProfile()
	{
		BotName = FString("Bot");
		bIsFemale = false;
		Accuracy = 0.0f;
		Alertness = 0.0f;
		PrimaryColor = FLinearColor(0.35f, 0.35f, 0.4f);
		SecondaryColor = FLinearColor(0.65f, 0.65f, 0.65f);
	}

	FBotProfile(const FString InName, const bool bInIsFemale)
	{
		BotName = InName;
		bIsFemale = bInIsFemale;
		Accuracy = 0.0f;
		Alertness = 0.0f;
		PrimaryColor = FLinearColor(0.35f, 0.35f, 0.4f);
		SecondaryColor = FLinearColor(0.65f, 0.65f, 0.65f);
		// Sexist Test: make women pink by default just so everyone doesn't have the same colours.
		if (bIsFemale)
		{
			PrimaryColor = FLinearColor(0.45f, 0.4f, 0.4f);
			SecondaryColor = FLinearColor(1.0f, 0.41f, 0.7f);
		}
	}
};

/**
 * The AI for bots that replace human players.
 */
UCLASS()
class LASTIM_API ASolBot : public ASolAIController
{
	GENERATED_UCLASS_BODY()
	
	/* Sets the bot's profile and adjusts bot's properties to match. */
	virtual void SetBotProfile(struct FBotProfile InProfile);

protected:
	/* The bot's profile. */
	struct FBotProfile BotProfile;
	
	
};
