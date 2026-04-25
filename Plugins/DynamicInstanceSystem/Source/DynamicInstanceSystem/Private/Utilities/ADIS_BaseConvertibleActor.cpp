#include "Utilities/ADIS_BaseConvertibleActor.h"
#include "Components/StaticMeshComponent.h"
#include "Data/DynamicConversionDefinition.h"
#include "Utilities/DynamicInstanceUtilities.h"

ADIS_BaseConvertibleActor::ADIS_BaseConvertibleActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetMobility(EComponentMobility::Movable);
}

void ADIS_BaseConvertibleActor::OnConvertedFromInstance_Implementation(const FTransform& InstanceTransform, const FInstancedStruct& SavedState)
{
	InternalSavedState = SavedState;
	SetActorTransform(InstanceTransform);

	DI_LOG(Log, TEXT("Actor %s Swapped In at %s"), *GetName(), *InstanceTransform.GetLocation().ToString());
}

void ADIS_BaseConvertibleActor::ApplyDefinitionVisuals(const UDynamicConversionDefinition* Definition)
{
	if (Definition && MeshComponent && Definition->DefaultMesh)
	{
		MeshComponent->SetStaticMesh(Definition->DefaultMesh);
	}
}

void ADIS_BaseConvertibleActor::PrepareForReversion_Implementation(FInstancedStruct& OutStateToSave)
{
	OutStateToSave = InternalSavedState;
	UE_LOG(LogTemp, Warning, TEXT("ACTOR SLEEP: %s is saving state and reverting."), *GetName());
}

void ADIS_BaseConvertibleActor::SimulateInteraction()
{
	SetActorScale3D(GetActorScale3D() * 0.8f);
	UE_LOG(LogTemp, Log, TEXT("ACTOR INTERACT: %s modified!"), *GetName());
}