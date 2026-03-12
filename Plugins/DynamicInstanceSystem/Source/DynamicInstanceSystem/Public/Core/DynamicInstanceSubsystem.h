#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/DynamicInstanceTypes.h"
#include "DynamicInstanceSubsystem.generated.h"

class UDynamicConversionDefinition;
class UDynamicInstanceSourceComponent;

UCLASS()
class DYNAMICINSTANCESYSTEM_API UDynamicInstanceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void OnUpdate(float DeltaTime);
	void RegisterSourceComponent(UDynamicInstanceSourceComponent* Component);
	void UnregisterSourceComponent(UDynamicInstanceSourceComponent* Component);
	bool IsInstanceConverted(const FDynamicInstanceKey& Key) const;
	AActor* GetActorForKey(const FDynamicInstanceKey& Key) const;
	int32 GetRegistrySize () const; 
	bool ManualConvert(UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);
	bool ManualRevert(UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);
	
protected:
	UPROPERTY()
	TMap<FDynamicInstanceKey, FDynamicInstanceRecord> InstanceRegistry;

	// Core logic
	bool ConvertInstanceToActor(FDynamicInstanceKey Key, FDynamicInstanceRecord& Record);
	bool RevertActorToInstance(FDynamicInstanceKey Key, FDynamicInstanceRecord& Record);

private:
	UPROPERTY()
	TSet<TObjectPtr<UDynamicInstanceSourceComponent>> RegisteredSources;

	FTimerHandle UpdateTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float UpdateInterval = 0.2f;

	void OnUpdateTimer();
	void EvaluateSources();
	bool ConvertInstance(UDynamicInstanceSourceComponent* Source, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex);
	bool RevertInstance(const FDynamicInstanceKey& Key);
	bool GetPlayerLocation(FVector& OutLocation) const;
	bool GetPrimaryQueryLocation(FVector& OutLocation) const;
	bool HasSatisfiedHysteresis(const FDynamicInstanceRecord& Record, const UDynamicConversionDefinition* Def) const;
	void PruneStaleRecords();
	bool ShouldConvertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const;
	bool ShouldRevertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const;
	
};