// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "LastimSpectatorPawn.h"

void ALastimSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ALastimSpectatorPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ALastimSpectatorPawn::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ADefaultPawn::MoveUp_World);
	InputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ADefaultPawn::LookUpAtRate);

	//InputComponent->BindAxis("Jump", IE_Pressed, this, &ALastimSpectatorPawn::MoveUp_World, 1.f);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analogue joystick
	//InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//InputComponent->BindAxis("TurnRate", this, &ALastimCharacter::TurnAtRate);
	//InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	//InputComponent->BindAxis("LookUpRate", this, &ALastimCharacter::LookUpAtRate);
}


