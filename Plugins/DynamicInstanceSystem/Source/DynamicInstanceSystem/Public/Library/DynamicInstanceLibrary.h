#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/DynamicInstanceTypes.h"
#include "DynamicInstanceLibrary.generated.h"

class UDynamicInstanceSubsystem;
class UInstancedStaticMeshComponent;

UCLASS()
class DYNAMICINSTANCESYSTEM_API UDynamicInstanceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if the specific instance is currently represented by a spawned actor */
	UFUNCTION(BlueprintPure, Category = "Dynamic Instance", meta = (WorldContext = "WorldContextObject"))
	static bool IsInstanceConverted(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);

	/** Returns the Actor associated with a converted instance, or null if it's still an instance */
	UFUNCTION(BlueprintPure, Category = "Dynamic Instance", meta = (WorldContext = "WorldContextObject"))
	static AActor* GetActorForInstance(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);

	/** Manually forces an instance to convert to an actor, ignoring distance and hysteresis */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance", meta = (WorldContext = "WorldContextObject"))
	static bool ForceConvertInstance(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);

	/** Manually forces a dynamic actor back into its instance form */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance", meta = (WorldContext = "WorldContextObject"))
	static bool ForceRevertInstance(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);

	/** Gets the total number of objects currently being tracked by the system */
	UFUNCTION(BlueprintPure, Category = "Dynamic Instance", meta = (WorldContext = "WorldContextObject"))
	static int32 GetTrackedRecordCount(const UObject* WorldContextObject);

private:
	/** Internal helper to safely get the subsystem */
	static UDynamicInstanceSubsystem* GetSubsystem(const UObject* WorldContextObject);
};