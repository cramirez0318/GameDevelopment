#pragma once

#include "CoreMinimal.h"
#include "InstancedStruct.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "StructUtils/InstancedStruct.h"
#include "DynamicInstanceTypes.generated.h"

USTRUCT(BlueprintType)
struct FDynamicInstanceKey
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Instance Conversion")
    TWeakObjectPtr<UInstancedStaticMeshComponent> SourceComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Instance Conversion")
    int32 InstanceIndex = -1;

    FDynamicInstanceKey() : SourceComponent(nullptr), InstanceIndex(-1) {}
    FDynamicInstanceKey(UInstancedStaticMeshComponent* InComponent, int32 InIndex) 
       : SourceComponent(InComponent), InstanceIndex(InIndex) {}

    bool IsValid() const { return SourceComponent.IsValid() && InstanceIndex != -1; }
    bool operator==(const FDynamicInstanceKey& Other) const { return SourceComponent == Other.SourceComponent && InstanceIndex == Other.InstanceIndex; }
};

FORCEINLINE uint32 GetTypeHash(const FDynamicInstanceKey& Key)
{
    uint32 Hash = 0;
    Hash = HashCombine(Hash, GetTypeHash(Key.SourceComponent));
    Hash = HashCombine(Hash, GetTypeHash(Key.InstanceIndex));
    return Hash;
}

USTRUCT(BlueprintType)
struct FDynamicInstanceRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Instance Conversion")
    FDynamicInstanceKey Key;

    UPROPERTY(BlueprintReadOnly, Category = "Instance Conversion")
    TWeakObjectPtr<class UDynamicInstanceSourceComponent> SourceComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Instance Conversion")
    TWeakObjectPtr<AActor> SpawnedActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Instance Conversion")
    FTransform OriginalInstanceTransform = FTransform::Identity;

    UPROPERTY(BlueprintReadOnly, Category = "Instance Conversion")
    FInstancedStruct SavedGameplayState;

    UPROPERTY(BlueprintReadOnly, Category = "Instance Conversion")
    double LastStateChangeTime = 0.0;

    UPROPERTY(BlueprintReadOnly, Category = "Instance Conversion")
    bool bIsConverted = false;

    FDynamicInstanceRecord() = default;
};