// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolSpectatorPawn.h"

ASolSpectatorPawn::ASolSpectatorPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	/*UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	Camera->AttachTo(this->GetRootComponent());
	Camera->SetRelativeLocation(FVector(-50.0, 0.0f, 0.0f));
	Camera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;*/
}

void ASolSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ADefaultPawn::LookUpAtRate);
	InputComponent->BindAxis("Bank", this, &ADefaultPawn::AddControllerRollInput);
}

void ASolSpectatorPawn::MoveUp(float Value)
{
	if (Value != 0.f)
	{
		if (Controller)
		{
			FRotator const ControlSpaceRot = Controller->GetControlRotation();

			// Move up relative to our rotation if we can roll.
			// Otherwise, just go up relative to the world.
			//const FVector UpVector = bUseControllerRotationRoll ? FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Z) : FVector::UpVector;
			const FVector UpVector = FRotationMatrix(ControlSpaceRot).GetScaledAxis(EAxis::Z);
			AddMovementInput(UpVector, Value);
		}
	}

	
}
