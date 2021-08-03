// Copyright Kyle Taylor Lange

#include "Lastim.h"
#include "UsableObjectInterface.h"

bool IUsableObjectInterface::CanBeUsedBy(AActor* User)
{
	return false;
}

bool IUsableObjectInterface::OnStartUseBy(AActor* User)
{
	return false;
}

bool IUsableObjectInterface::OnStopUseBy(AActor* User)
{
	return false;
}

FString IUsableObjectInterface::GetUseActionName(AActor* User)
{
	return FString("Generic Use Action");
}