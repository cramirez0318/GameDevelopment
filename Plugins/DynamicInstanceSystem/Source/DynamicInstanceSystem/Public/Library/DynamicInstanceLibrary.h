#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/DynamicInstanceTypes.h"
#include "DynamicInstanceLibrary.generated.h"

class UDynamicConversionDefinition;
class UDynamicInstanceSubsystem;
class UInstancedStaticMeshComponent;

UENUM(BlueprintType)
enum class EDIS_ConversionOutcome : uint8
{
	Success,
	Failure
};

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
	
	/** --- STATE HELPERS --- */

	/** Extracts custom data from an InstancedStruct into a Blueprint struct (Wildcard) */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Dynamic Instance|State", meta = (CustomStructureParam = "OutStruct", ExpandEnumAsExecs = "Outcome"))
	static void GetStateData(const FInstancedStruct& InstancedStruct, int32& OutStruct, EDIS_ConversionOutcome& Outcome);

	/** Packs a Blueprint struct into an InstancedStruct for saving (Wildcard) */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Dynamic Instance|State", meta = (CustomStructureParam = "InStruct"))
	static void SetStateData(UPARAM(ref) FInstancedStruct& InstancedStruct, const int32& InStruct);
	
	/** * The "One-Click" setup for the system. 
	 * Registers the player, sets update rates, and toggles debug visuals.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance|System", meta = (WorldContext = "WorldContextObject"))
	static void InitializeDynamicInstanceSystem(
		const UObject* WorldContextObject, 
		bool bEnableDebug = true, 
		float UpdateFrequency = 0.1f, 
		bool bAutoRegisterPlayer = true
	);
	
	/** * Sets a float value inside an InstancedStruct by property name.
	 * Useful for 'Health' or 'Durability' during testing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance|Testing")
	static void SetFloatInState(UPARAM(ref) FInstancedStruct& State, FName PropertyName, float Value);

	/** * Gets a float value from an InstancedStruct by property name. */
	UFUNCTION(BlueprintPure, Category = "Dynamic Instance|Testing")
	static float GetFloatFromState(const FInstancedStruct& State, FName PropertyName);
	
	/** * Scans the level for StaticMeshActors with a specific Tag and converts 
	 * them into a single managed ISM Source for the Subsystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance|Setup", meta = (WorldContext = "WorldContextObject"))
	static void BakeStaticMeshesToSystem(const UObject* WorldContextObject, FName ActorTag, UDynamicConversionDefinition* Definition);

	// Thunk Implementations
	DECLARE_FUNCTION(execGetStateData);
	DECLARE_FUNCTION(execSetStateData);

private:
	/** Internal helper to safely get the subsystem */
	static UDynamicInstanceSubsystem* GetSubsystem(const UObject* WorldContextObject);
};