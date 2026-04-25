#include "Actors/DIS_InstanceSourceActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DynamicInstanceSourceComponent.h"
#include "Data/DynamicConversionDefinition.h"
#include "Utilities/DynamicInstanceUtilities.h"

ADIS_InstanceSourceActor::ADIS_InstanceSourceActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InstancedMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMeshComponent"));
	SetRootComponent(InstancedMeshComponent);

	SourceComponent = CreateDefaultSubobject<UDynamicInstanceSourceComponent>(TEXT("DynamicInstanceSourceComponent"));
}

void ADIS_InstanceSourceActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BuildFromDefinition();
}

void ADIS_InstanceSourceActor::BeginPlay()
{
	Super::BeginPlay();
}

void ADIS_InstanceSourceActor::BuildFromDefinition()
{
	if (!InstancedMeshComponent || !SourceComponent)
	{
		return;
	}

	InstancedMeshComponent->ClearInstances();

	if (!ConversionDefinition)
	{
		DI_LOG(Warning, TEXT("DIS_InstanceSourceActor [%s] has no ConversionDefinition assigned."), *GetName());
		return;
	}

	if (!ConversionDefinition->DefaultMesh)
	{
		DI_LOG(Warning, TEXT("DIS_InstanceSourceActor [%s] has a definition with no DefaultMesh."), *GetName());
		return;
	}

	InstancedMeshComponent->SetStaticMesh(ConversionDefinition->DefaultMesh);
	SourceComponent->SetConversionDefinition(ConversionDefinition);

	for (const FInstancePlacementData& Placement : ConversionDefinition->ManualPlacements)
	{
		InstancedMeshComponent->AddInstance(Placement.Transform);
	}

	DI_LOG(Log, TEXT("DIS_InstanceSourceActor [%s] built %d instances from definition [%s]."),
		*GetName(),
		ConversionDefinition->ManualPlacements.Num(),
		*ConversionDefinition->GetName());
}