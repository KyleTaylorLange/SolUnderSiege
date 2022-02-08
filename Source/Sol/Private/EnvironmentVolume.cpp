// Copyright Kyle Taylor Lange

#include "Sol.h"
#include "EnvironmentVolume.h"
#include "SolWorldSettings.h"


//////////////////////////////////////////////////////////////////////////
// AEnvironmentVolume

AEnvironmentVolume::AEnvironmentVolume(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GravityZ = 0.0f;
	Temperature = 0.0f;
	Pressure = -1.0f;
}

float AEnvironmentVolume::GetGravityZ() const
{
	if (GravityZ == 0.0f)
	{
		return Super::GetGravityZ();
	}
	else
	{
		return GravityZ;
	}
}

float AEnvironmentVolume::GetTemperature() const
{
	if (Temperature == 0.0f)
	{
		ASolWorldSettings* Settings = Cast<ASolWorldSettings>(GetWorld()->GetWorldSettings());
		if (Settings)
		{
			return Settings->GetTemperature();
		}
	}
	return Temperature;
}

float AEnvironmentVolume::GetPressure() const
{
	if (Pressure < 0.0f)
	{
		ASolWorldSettings* Settings = Cast<ASolWorldSettings>(GetWorld()->GetWorldSettings());
		if (Settings)
		{
			return Settings->GetPressure();
		}
	}
	return Pressure;
}