// Copyright Kyle Taylor Lange

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UsableObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UUsableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for objects that can be used by players in the world (i.e. responds to pressing the use key).
 */
class LASTIM_API IUsableObjectInterface
{
	GENERATED_BODY()

public:

	// Can the incoming user use this item?
	//UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual bool CanBeUsedBy(AActor* User);

	/** When someone starts using this item (i.e. use is held down). */
	//UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual bool OnStartUseBy(AActor* User);

	/** When someone stops using this item (i.e. use is released). */
	//UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual bool OnStopUseBy(AActor* User);

	// Returns a message about what this use action does.
	//UFUNCTION(BlueprintCallable, Category = Gameplay)
	virtual FString GetUseActionName(AActor* User);
};
