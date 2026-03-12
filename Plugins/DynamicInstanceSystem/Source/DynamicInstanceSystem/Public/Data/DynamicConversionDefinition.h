#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Actor.h"
#include "DynamicConversionDefinition.generated.h"

UCLASS(BlueprintType)
class DYNAMICINSTANCESYSTEM_API UDynamicConversionDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (UIMin = "0"))
	float ConversionRadius = 1500.0f;

	/** Must be larger than ConversionRadius to prevent thrashing */
	UPROPERTY(EditAnywhere, Category = "Settings", meta = (UIMin = "0"))
	float RevertRadius = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (UIMin = "0"))
	float HysteresisTime = 2.0f;

	/** Manual check for the Subsystem */
	bool IsValidDefinition() const 
	{
		return ActorClass != nullptr && RevertRadius > ConversionRadius;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		if (RevertRadius <= ConversionRadius)
		{
			RevertRadius = ConversionRadius + 500.0f;
		}
	}
#endif
};