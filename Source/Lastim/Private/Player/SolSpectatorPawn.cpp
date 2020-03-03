// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "SolSpectatorPawn.h"

void ASolSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ASolSpectatorPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASolSpectatorPawn::MoveRight);
	//InputComponent->BindAxis("MoveUp", this, &ADefaultPawn::MoveUp_World);
	InputComponent->BindAxis("Jump", this, &ASolSpectatorPawn::MoveUp_World);
	InputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ADefaultPawn::LookUpAtRate);
}


