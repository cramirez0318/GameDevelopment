#include "Components/DynamicInstanceSourceComponent.h"
#include "Core/DynamicInstanceSubsystem.h"
#include "Data/DynamicConversionDefinition.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"

UDynamicInstanceSourceComponent::UDynamicInstanceSourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false; 
}

void UDynamicInstanceSourceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UDynamicInstanceSubsystem* Subsystem = World->GetSubsystem<UDynamicInstanceSubsystem>())
		{
			Subsystem->RegisterSourceComponent(this);
		}
	}
}

void UDynamicInstanceSourceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UDynamicInstanceSubsystem* Subsystem = World->GetSubsystem<UDynamicInstanceSubsystem>())
		{
			Subsystem->UnregisterSourceComponent(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool UDynamicInstanceSourceComponent::HasValidDefinition() const
{
	return ConversionDefinition != nullptr && ConversionDefinition->IsValidDefinition();
}

TArray<UInstancedStaticMeshComponent*> UDynamicInstanceSourceComponent::GetSourceComponents() const
{
	return GetRegisteredISMComponents();
}

TArray<UInstancedStaticMeshComponent*> UDynamicInstanceSourceComponent::GetRegisteredISMComponents() const
{
	TArray<UInstancedStaticMeshComponent*> OutComponents;
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return OutComponents;
	}

	if (FilterMode == EDynamicInstanceFilterMode::ExplicitList)
	{
		for (UInstancedStaticMeshComponent* ISM : ExplicitComponents)
		{
			if (IsValid(ISM))
			{
				OutComponents.Add(ISM);
			}
		}
		return OutComponents;
	}

	TArray<UInstancedStaticMeshComponent*> FoundComponents;
	Owner->GetComponents<UInstancedStaticMeshComponent>(FoundComponents);

	for (UInstancedStaticMeshComponent* ISM : FoundComponents)
	{
		if (!IsValid(ISM))
		{
			continue;
		}

		if (FilterMode == EDynamicInstanceFilterMode::TagFilter &&
			!ISM->ComponentHasTag(RequiredComponentTag))
		{
			continue;
		}

		OutComponents.Add(ISM);
	}

	return OutComponents;
}

void UDynamicInstanceSourceComponent::ForEachISMComponent(TFunctionRef<void(UInstancedStaticMeshComponent*)> Func)
{
	TArray<UInstancedStaticMeshComponent*> Components = GetRegisteredISMComponents();
	for (UInstancedStaticMeshComponent* ISM : Components)
	{
		Func(ISM);
	}
}

bool UDynamicInstanceSourceComponent::GetInstanceWorldTransform(UInstancedStaticMeshComponent* ISM, int32 Index, FTransform& OutTransform) const
{
	if (IsValid(ISM) && ISM->IsValidInstance(Index))
	{
		return ISM->GetInstanceTransform(Index, OutTransform, true);
	}
	return false;
}

bool UDynamicInstanceSourceComponent::HideInstance(UInstancedStaticMeshComponent* ISM, int32 Index)
{
	if (!IsValid(ISM) || !ISM->IsValidInstance(Index))
	{
		return false;
	}

	FTransform HiddenTransform;
	if (!ISM->GetInstanceTransform(Index, HiddenTransform, true))
	{
		return false;
	}

	HiddenTransform.SetScale3D(FVector::ZeroVector);

	return ISM->UpdateInstanceTransform(
		Index,
		HiddenTransform,
		true,
		true,
		true
	);
}

bool UDynamicInstanceSourceComponent::RestoreInstance(UInstancedStaticMeshComponent* ISM, int32 Index, const FTransform& OriginalTransform)
{
	if (IsValid(ISM) && ISM->IsValidInstance(Index))
	{
		return ISM->UpdateInstanceTransform(
			Index, 
			OriginalTransform, 
			true, 
			true, 
			true
		);
	}
	return false;
}