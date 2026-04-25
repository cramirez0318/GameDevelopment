#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IDynamicConvertibleActor.h"
#include "ADIS_BaseConvertibleActor.generated.h"

class UDynamicConversionDefinition;

UCLASS(Blueprintable)
class DYNAMICINSTANCESYSTEM_API ADIS_BaseConvertibleActor : public AActor, public IDynamicConvertibleActor
{
	GENERATED_BODY()

public:
	ADIS_BaseConvertibleActor();

	/** The visual representation of the actor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Persistent State */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Instance Conversion")
	FInstancedStruct InternalSavedState;
	
	UFUNCTION(BlueprintCallable, Category="Setup")
	void ApplyDefinitionVisuals(const UDynamicConversionDefinition* Definition);

	// --- INTERFACE ---
	virtual void OnConvertedFromInstance_Implementation(const FTransform& InstanceTransform, const FInstancedStruct& SavedState) override;
	virtual void PrepareForReversion_Implementation(FInstancedStruct& OutStateToSave) override;
	virtual bool CanRevertToInstance_Implementation() const override { return true; }

	/** Simple debug function to modify state (call via 'G' key or similar in BP) */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void SimulateInteraction();
};