
#include "Library/DynamicInstanceLibrary.h"
#include "Core/DynamicInstanceSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Components/DynamicInstanceSourceComponent.h"
#include "Data/DynamicConversionDefinition.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/DynamicInstanceUtilities.h"

UDynamicInstanceSubsystem* UDynamicInstanceLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetSubsystem<UDynamicInstanceSubsystem>();
	}
	return nullptr;
}

/** * SET STATE DATA - Implementation
 */
DEFINE_FUNCTION(UDynamicInstanceLibrary::execSetStateData)
{
	P_GET_STRUCT_REF(FInstancedStruct, InstancedStruct);
	
	Stack.StepCompiledIn<FStructProperty>(NULL);
	FStructProperty* InStructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* InStructPtr = Stack.MostRecentPropertyAddress;

	P_FINISH;

	P_NATIVE_BEGIN;
	if (InStructProp && InStructPtr)
	{
		InstancedStruct.InitializeAs(InStructProp->Struct, (uint8*)InStructPtr);
	}
	P_NATIVE_END;
}

DEFINE_FUNCTION(UDynamicInstanceLibrary::execGetStateData)
{
	P_GET_STRUCT_REF(FInstancedStruct, InstancedStruct);

	Stack.StepCompiledIn<FStructProperty>(NULL);
	FStructProperty* OutStructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* OutStructPtr = Stack.MostRecentPropertyAddress;

	P_GET_ENUM_REF(EDIS_ConversionOutcome, Outcome);
	P_FINISH;

	P_NATIVE_BEGIN;
	bool bSuccess = false;
	if (OutStructProp && OutStructPtr && InstancedStruct.IsValid())
	{
		if (InstancedStruct.GetScriptStruct()->IsChildOf(OutStructProp->Struct))
		{
			OutStructProp->Struct->CopyScriptStruct(OutStructPtr, InstancedStruct.GetMemory());
			bSuccess = true;
		}
	}
	Outcome = bSuccess ? EDIS_ConversionOutcome::Success : EDIS_ConversionOutcome::Failure;
	P_NATIVE_END;
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

void UDynamicInstanceLibrary::InitializeDynamicInstanceSystem(const UObject* WorldContextObject, bool bEnableDebug, float UpdateFrequency, bool bAutoRegisterPlayer)
{
	UDynamicInstanceSubsystem* Subsystem = GetSubsystem(WorldContextObject);
	if (!Subsystem) return;

	Subsystem->bEnableDebugDraw = bEnableDebug;
	Subsystem->bEnableDebugLogging = bEnableDebug;
	Subsystem->UpdateInterval = UpdateFrequency;
	Subsystem->RefreshUpdateTimer(); 

	if (bAutoRegisterPlayer)
	{
		UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
		if (PlayerPawn)
		{
			Subsystem->RegisterQueryActor(PlayerPawn);
		}
	}

	DI_LOG(Log, TEXT("System Initialized via Helper Node. Debug: %s, Frequency: %f"), bEnableDebug ? TEXT("ON") : TEXT("OFF"), UpdateFrequency);
}

void UDynamicInstanceLibrary::SetFloatInState(FInstancedStruct& State, FName PropertyName, float Value)
{
	if (!State.IsValid()) return;

	if (FNumericProperty* NumProp = CastField<FNumericProperty>(State.GetScriptStruct()->FindPropertyByName(PropertyName)))
	{
		uint8* MutableMemory = const_cast<uint8*>(State.GetMemory());
		NumProp->SetFloatingPointPropertyValue(MutableMemory, Value);
	}
}

float UDynamicInstanceLibrary::GetFloatFromState(const FInstancedStruct& State, FName PropertyName)
{
	if (!State.IsValid()) return 0.0f;
    
	if (FNumericProperty* NumProp = CastField<FNumericProperty>(State.GetScriptStruct()->FindPropertyByName(PropertyName)))
	{
		return NumProp->GetFloatingPointPropertyValue(State.GetMemory());
	}
	return 0.0f;
}

void UDynamicInstanceLibrary::BakeStaticMeshesToSystem(const UObject* WorldContextObject, FName ActorTag, UDynamicConversionDefinition* Definition)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !Definition || !Definition->DefaultMesh) 
	{
		UE_LOG(LogTemp, Error, TEXT("Bake failed: Invalid World, Definition, or DefaultMesh missing!"));
		return;
	}
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(World, ActorTag, FoundActors);
	if (FoundActors.Num() == 0) return;
	AActor* GhostManager = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
	GhostManager->SetActorLabel(FString::Printf(TEXT("ManagedSource_%s"), *ActorTag.ToString()));
	UInstancedStaticMeshComponent* ISM = NewObject<UInstancedStaticMeshComponent>(GhostManager);
	ISM->RegisterComponent();
	ISM->SetStaticMesh(Definition->DefaultMesh);
	GhostManager->AddInstanceComponent(ISM);
	UDynamicInstanceSourceComponent* Source = NewObject<UDynamicInstanceSourceComponent>(GhostManager);
	Source->RegisterComponent();
	Source->SetConversionDefinition(Definition);
	GhostManager->AddInstanceComponent(Source);
	for (AActor* Actor : FoundActors)
	{
		ISM->AddInstance(Actor->GetActorTransform(), true);
		Actor->Destroy(); 
	}

	DI_LOG(Log, TEXT("Baking Complete: Converted %d Actors into 1 Managed Source."), FoundActors.Num());
}