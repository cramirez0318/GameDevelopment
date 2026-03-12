#include "Library/DynamicInstanceLibrary.h"
#include "Core/DynamicInstanceSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDynamicInstanceSubsystem* UDynamicInstanceLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UDynamicInstanceSubsystem>();
	}
	return nullptr;
}

bool UDynamicInstanceLibrary::IsInstanceConverted(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	if (UDynamicInstanceSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->IsInstanceConverted(FDynamicInstanceKey(ISM, InstanceIndex));
	}
	return false;
}

AActor* UDynamicInstanceLibrary::GetActorForInstance(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	if (UDynamicInstanceSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetActorForKey(FDynamicInstanceKey(ISM, InstanceIndex));
	}
	return nullptr;
}

bool UDynamicInstanceLibrary::ForceConvertInstance(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	if (UDynamicInstanceSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->ManualConvert(ISM, InstanceIndex);
	}
	return false;
}

bool UDynamicInstanceLibrary::ForceRevertInstance(const UObject* WorldContextObject, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	if (UDynamicInstanceSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->ManualRevert(ISM, InstanceIndex);
	}
	return false;
}

int32 UDynamicInstanceLibrary::GetTrackedRecordCount(const UObject* WorldContextObject)
{
	if (UDynamicInstanceSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetRegistrySize();
	}
	return 0;
}
