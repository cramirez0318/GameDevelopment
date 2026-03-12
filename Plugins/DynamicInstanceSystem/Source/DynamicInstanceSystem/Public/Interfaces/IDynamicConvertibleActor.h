// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Interface.h"
#include "IDynamicConvertibleActor.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDynamicConvertibleActor : public UInterface
{
	GENERATED_BODY()
};

class DYNAMICINSTANCESYSTEM_API IDynamicConvertibleActor
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Conversion")
	void OnConvertedFromInstance(const FTransform& InstanceTransform, const FInstancedStruct& SavedState);

	UFUNCTION(BlueprintNativeEvent, Category = "Conversion")
	void PrepareForReversion(UPARAM(ref) FInstancedStruct& OutStateToSave);

	UFUNCTION(BlueprintNativeEvent, Category = "Conversion")
	bool CanRevertToInstance() const;
};