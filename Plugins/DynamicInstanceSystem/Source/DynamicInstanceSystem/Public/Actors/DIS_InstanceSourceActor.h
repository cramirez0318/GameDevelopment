#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DIS_InstanceSourceActor.generated.h"

class UInstancedStaticMeshComponent;
class UDynamicInstanceSourceComponent;
class UDynamicConversionDefinition;

UCLASS(Blueprintable)
class DYNAMICINSTANCESYSTEM_API ADIS_InstanceSourceActor : public AActor
{
	GENERATED_BODY()

public:
	ADIS_InstanceSourceActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInstancedStaticMeshComponent> InstancedMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDynamicInstanceSourceComponent> SourceComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TObjectPtr<UDynamicConversionDefinition> ConversionDefinition;

	UFUNCTION(BlueprintCallable, Category = "Setup")
	void BuildFromDefinition();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
};