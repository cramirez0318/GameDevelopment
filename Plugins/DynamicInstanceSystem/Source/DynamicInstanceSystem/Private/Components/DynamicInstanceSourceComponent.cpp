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
	if (AActor* Owner = GetOwner())
	{
		Owner->GetComponents<UInstancedStaticMeshComponent>(OutComponents);
	}
	return OutComponents;
}

void UDynamicInstanceSourceComponent::ForEachISMComponent(TFunctionRef<void(UInstancedStaticMeshComponent*)> Func) const
{
	if (AActor* Owner = GetOwner())
	{
		TArray<UInstancedStaticMeshComponent*> Components;
		Owner->GetComponents<UInstancedStaticMeshComponent>(Components);
		for (UInstancedStaticMeshComponent* Comp : Components)
		{
			if (IsValid(Comp))
			{
				Func(Comp);
			}
		}
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
	if (IsValid(ISM) && ISM->IsValidInstance(Index))
	{
		FTransform HiddenTransform = FTransform::Identity;
		HiddenTransform.SetScale3D(FVector::ZeroVector);
		return ISM->UpdateInstanceTransform(
			Index, 
			HiddenTransform, 
			true,
			true,
			true
		);
	}
	return false;
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