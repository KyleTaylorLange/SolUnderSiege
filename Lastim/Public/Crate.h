// Copyright Kyle Taylor Lange

#pragma once

#include "LastimDestructible.h"
#include "Crate.generated.h"

/**
 * 
 */
UCLASS()
class LASTIM_API ACrate : public ALastimDestructible
{
	GENERATED_BODY()
	
	/** The crate's mesh. **/
	UPROPERTY(VisibleAnywhere, Category = Mesh) //BlueprintReadOnly or BlueprintReadWrite?
	UStaticMeshComponent* Mesh;
	
public:

	ACrate(const FObjectInitializer& ObjectInitializer);
};
