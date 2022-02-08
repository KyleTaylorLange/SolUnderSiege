// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolBot.h"

ASolBot::ASolBot(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bWantsPlayerState = true;
	bCanPickUpItems = true;
}

void ASolBot::SetBotProfile(struct FBotProfile InProfile)
{
	BotProfile = InProfile;

	/* Temporary to make alertness affect something. */
	PeripheralVision = 0.5f - (0.25f * BotProfile.Alertness);
	RotationSpeed = 270.f + (90 * BotProfile.Alertness);

	MaxTargetPredictionError = 0.1f + (0.3f * FMath::Abs(BotProfile.Accuracy - 1.0f));
	MaxAimOffsetError = 0.1f + (0.3f * FMath::Abs(BotProfile.Accuracy - 1.0f));
	MaxRecoilCompensationError = 0.1f + (0.3f * FMath::Abs(BotProfile.Accuracy - 1.0f));
}
