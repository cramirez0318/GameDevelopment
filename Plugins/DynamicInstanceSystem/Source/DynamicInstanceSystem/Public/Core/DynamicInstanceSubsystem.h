#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Data/DynamicInstanceTypes.h"
#include "DynamicInstanceSubsystem.generated.h"

class UDynamicConversionDefinition;
class UDynamicInstanceSourceComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnInstanceConverted,
	UInstancedStaticMeshComponent*, InstancedMeshComponent,
	int32, InstanceIndex,
	AActor*, ConvertedActor
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnInstanceReverted,
	UInstancedStaticMeshComponent*, InstancedMeshComponent,
	int32, InstanceIndex
);

UCLASS(Config = Game, DefaultConfig)
class DYNAMICINSTANCESYSTEM_API UDynamicInstanceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterSourceComponent(UDynamicInstanceSourceComponent* Component);
	void UnregisterSourceComponent(UDynamicInstanceSourceComponent* Component);

	UPROPERTY(BlueprintAssignable, Category = "Dynamic Instance|Events")
	FOnInstanceConverted OnInstanceConverted;

	UPROPERTY(BlueprintAssignable, Category = "Dynamic Instance|Events")
	FOnInstanceReverted OnInstanceReverted;

	UFUNCTION(BlueprintPure, Category = "Dynamic Instance")
	bool IsInstanceConverted(const FDynamicInstanceKey& Key) const;

	UFUNCTION(BlueprintPure, Category = "Dynamic Instance")
	AActor* GetActorForKey(const FDynamicInstanceKey& Key) const;

	UFUNCTION(BlueprintPure, Category = "Dynamic Instance")
	int32 GetRegistrySize() const;

	bool ManualConvert(UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);
	bool ManualRevert(UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);
	void RefreshUpdateTimer();

	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance")
	void RegisterQueryActor(AActor* QueryActor);

	UFUNCTION(BlueprintCallable, Category = "Dynamic Instance")
	void UnregisterQueryActor(AActor* QueryActor);
	
	/** How often the proximity check runs (in seconds). Lower is more responsive but heavier on CPU. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.01"))
	float UpdateInterval = 0.2f;

	/** If true, the subsystem will output state changes and errors to the DynamicInstance log category. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableDebugLogging = false;

	/** If true, draws spheres and circles in the viewport to visualize conversion ranges. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableDebugDraw = false;

	/** How long debug lines persist. Use 0.0 for single-frame (flicker-free) updates. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (ClampMin = "0.0"))
	float DebugDrawDuration = 0.0f;

protected:
	UPROPERTY()
	TMap<FDynamicInstanceKey, FDynamicInstanceRecord> InstanceRegistry;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> RegisteredQueryActors;

private:
	UPROPERTY()
	TSet<TObjectPtr<UDynamicInstanceSourceComponent>> RegisteredSources;

	FTimerHandle UpdateTimerHandle;
	
	void OnUpdate(float DeltaTime);
	void OnUpdateTimer();
	void EvaluateSources();
	void PruneStaleRecords();

	bool ConvertInstance(UDynamicInstanceSourceComponent* Source, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);
	bool RevertInstance(const FDynamicInstanceKey& Key);

	bool ConvertInstanceToActor(FDynamicInstanceKey Key, FDynamicInstanceRecord& Record);
	bool RevertActorToInstance(FDynamicInstanceKey Key, FDynamicInstanceRecord& Record);

	bool GetPrimaryQueryLocation(FVector& OutLocation) const;
	bool HasSatisfiedHysteresis(const FDynamicInstanceRecord& Record, const UDynamicConversionDefinition* Def) const;
	bool ShouldConvertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const;
	bool ShouldRevertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const;

	void GetCollectionOfQueryLocations(TArray<FVector>& OutLocations);
	bool IsSourceInBroadphaseRange(const UDynamicInstanceSourceComponent* Source, const TArray<FVector>& QueryLocations) const;
	void ProcessInstance(UDynamicInstanceSourceComponent* Source, UInstancedStaticMeshComponent* ISM, int32 Index, const TArray<FVector>& QueryLocations);
	void DrawDebugVisuals(const FVector& InstanceLoc, float DistSq, const UDynamicConversionDefinition* Def) const;
};