// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "SolWorldSettings.h"

ASolWorldSettings::ASolWorldSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GlobalTemperature = 273.15f;
	GlobalPressure = 1.f;
}

float ASolWorldSettings::GetTemperature() const
{
	return GlobalTemperature;
}

float ASolWorldSettings::GetPressure() const
{
	return GlobalPressure;
}