#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DynamicInstanceTestingLibrary.generated.h"

UCLASS()
class DYNAMICINSTANCESYSTEM_API UDynamicInstanceTestingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** * One-node setup for testing. 
	 * Enables debug draw, logging, and registers the local player as a query source.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance|Testing", meta = (WorldContext = "WorldContextObject"))
	static void SetupDefaultTestingEnvironment(const UObject* WorldContextObject, bool bEnableDebug = true, float UpdateFrequency = 0.1f);

	/** * Prints a summary of the current system state to the screen/log.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance|Testing", meta = (WorldContext = "WorldContextObject"))
	static void RunSystemDiagnostic(const UObject* WorldContextObject);
	
	/** * Forcefully converts the instance the player is currently looking at. 
	 * Great for verifying mesh alignment and state loading.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance|Testing", meta = (WorldContext = "WorldContextObject"))
	static void ForceConvertLookAtTarget(const UObject* WorldContextObject, float MaxDistance = 5000.0f);
};