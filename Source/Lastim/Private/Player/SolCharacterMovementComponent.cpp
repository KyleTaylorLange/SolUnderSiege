// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolCharacter.h"
#include "SolCharacterMovementComponent.h"

USolCharacterMovementComponent::USolCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	CrouchedHalfHeight = 56.25f;
	NavAgentProps.bCanCrouch = true;
	bCanWalkOffLedgesWhenCrouching = true;
	MaxSprintSpeed = 900.f; //750.f;
	MaxWalkSpeed = 450.f; //375.f;
	MaxWalkSpeedCrouched = 225.f;
	MaxAimingSpeed = 300.f;
	MaxWalkSpeedCrouched = 300.f; // 250.f;
	JumpZVelocity = 540.f; //Just enough to jump on a 150 centimetre tall crate.
	MaxAcceleration = 2500.f;
	BrakingDecelerationWalking = 300.f;
	BrakingDecelerationWalking = 2500.f;
}


float USolCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	// TODO: Better handling if player is aiming and crouching.
	const ASolCharacter* SolCharacterOwner = Cast<ASolCharacter>(PawnOwner);
	if (SolCharacterOwner)
	{
		if (SolCharacterOwner->IsAiming())
		{
			MaxSpeed *= MaxAimingSpeed / MaxWalkSpeed;
		}
		if (SolCharacterOwner->IsSprinting())
		{
			MaxSpeed *= MaxSprintSpeed / MaxWalkSpeed;
		}
		/**
		if (IsCrouching())
		{
			MaxSpeed *= MaxWalkSpeedCrouched / MaxWalkSpeed;
		}
		*/
	}
	return MaxSpeed;
}

bool USolCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	if (CharacterOwner && CharacterOwner->CanJump())
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			//Velocity.Z = JumpZVelocity;
			Velocity.Z = CalculateJumpVelocity();
			SetMovementMode(MOVE_Falling);
			return true;
		}
	}

	return false;
}

float USolCharacterMovementComponent::CalculateJumpVelocity() const
{
	float FinalJumpZVelocity = JumpZVelocity;
	// TEMP: Dampen jump height in low gravity just to make low gravity maps a little less ridiculous.
	// Jump height will be decreased once we implement mantling, so we can ditch this afterwards.
	if (GetGravityZ() >= -1000.f)
	{
		FinalJumpZVelocity *= FMath::Clamp((GetGravityZ() - 1000.f) / -2000.f, 0.5f, 1.0f);
	}

	return FinalJumpZVelocity;
}