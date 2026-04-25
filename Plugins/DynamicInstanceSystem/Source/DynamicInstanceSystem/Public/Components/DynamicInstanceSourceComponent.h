#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DynamicInstanceSourceComponent.generated.h"

class UDynamicConversionDefinition;
class UInstancedStaticMeshComponent;

UENUM(BlueprintType)
enum class EDynamicInstanceFilterMode : uint8
{
	AllComponents      UMETA(DisplayName = "All ISM Components"),
	ExplicitList       UMETA(DisplayName = "Explicit List"),
	TagFilter          UMETA(DisplayName = "Filter by Component Tag")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DYNAMICINSTANCESYSTEM_API UDynamicInstanceSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDynamicInstanceSourceComponent();
	
	UFUNCTION(BlueprintCallable, Category="Conversion")
	void SetConversionDefinition(UDynamicConversionDefinition* InDefinition) { ConversionDefinition = InDefinition; }
	
	UPROPERTY(EditAnywhere, Category = "Instance Conversion|Filter")
	EDynamicInstanceFilterMode FilterMode = EDynamicInstanceFilterMode::AllComponents;

	/** Only used if FilterMode is set to 'Explicit List' */
	UPROPERTY(EditAnywhere, Category = "Instance Conversion|Filter", meta = (EditCondition = "FilterMode == EDynamicInstanceFilterMode::ExplicitList"))
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> ExplicitComponents;

	/** Only used if FilterMode is set to 'Filter by Component Tag' */
	UPROPERTY(EditAnywhere, Category = "Instance Conversion|Filter", meta = (EditCondition = "FilterMode == EDynamicInstanceFilterMode::TagFilter"))
	FName RequiredComponentTag;

	/** Public API for the Subsystem */
	
	UFUNCTION(BlueprintPure, Category = "Conversion")
	UDynamicConversionDefinition* GetConversionDefinition() const { return ConversionDefinition; }
	
	/** Helper for the Subsystem to check if this source is ready to go */
	bool HasValidDefinition() const;
	
	/** Abstraction: The Subsystem doesn't need to know how we find ISMs */
	TArray<UInstancedStaticMeshComponent*> GetSourceComponents() const;

	/** Returns all ISM/HISM components this source is managing */
	UFUNCTION(BlueprintCallable, Category = "Conversion")
	TArray<UInstancedStaticMeshComponent*> GetRegisteredISMComponents() const;

	/** Executes a function for every managed ISM component (High performance) */
	void ForEachISMComponent(TFunctionRef<void(UInstancedStaticMeshComponent*)> Func);
	
	/** API for ISM Abstraction */

	/** Safely gets the world transform of a specific instance */
	UFUNCTION(BlueprintCallable, Category = "Conversion")
	bool GetInstanceWorldTransform(UInstancedStaticMeshComponent* ISM, int32 Index, FTransform& OutTransform) const;

	/** * Policy: HideInstance
	 * Currently implements "Scale-to-Zero" to ensure Instance Indices remain stable.
	 * This prevents the TMap keys in the Subsystem from becoming invalidated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Conversion|Policy")
	bool HideInstance(UInstancedStaticMeshComponent* ISM, int32 Index);

	/** * Policy: RestoreInstance
	 * Returns the instance to its original state/transform.
	 */
	UFUNCTION(BlueprintCallable, Category = "Conversion|Policy")
	bool RestoreInstance(UInstancedStaticMeshComponent* ISM, int32 Index, const FTransform& OriginalTransform);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conversion")
	TObjectPtr<UDynamicConversionDefinition> ConversionDefinition;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};