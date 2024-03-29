// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
// Adapted by Kyle Taylor Lange from "ShooterGame.cpp" from the ShooterGame example project.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

class FSolSlateStyle
{
public:

	static void Initialize();

	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the Shooter game */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:

	static TSharedRef< class FSlateStyleSet > Create();

private:

	static TSharedPtr< class FSlateStyleSet >  SolSlateStyleInstance;
};