#include "Core/DynamicInstanceSubsystem.h"
#include "Components/DynamicInstanceSourceComponent.h"
#include "Data/DynamicConversionDefinition.h"
#include "Interfaces/IDynamicConvertibleActor.h"
#include "Utilities/DynamicInstanceUtilities.h"
#include "GameFramework/Pawn.h"
#include "Math/Color.h"
#include "DrawDebugHelpers.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UDynamicInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			UpdateTimerHandle, 
			this, 
			&UDynamicInstanceSubsystem::OnUpdateTimer, 
			UpdateInterval, 
			true
		);
	}
}

void UDynamicInstanceSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	Super::Deinitialize();
}

void UDynamicInstanceSubsystem::RegisterSourceComponent(UDynamicInstanceSourceComponent* Component)
{
	if (Component)
	{
		RegisteredSources.Add(Component);
	}
}

void UDynamicInstanceSubsystem::UnregisterSourceComponent(UDynamicInstanceSourceComponent* Component)
{
	if (Component)
	{
		RegisteredSources.Remove(Component);
	}
}

void UDynamicInstanceSubsystem::OnUpdateTimer()
{
	OnUpdate(UpdateInterval);
}

void UDynamicInstanceSubsystem::OnUpdate(float DeltaTime)
{
	FVector QueryLoc;
	if (!GetPrimaryQueryLocation(QueryLoc))
	{
		return;
	}

	EvaluateSources();
	PruneStaleRecords();
}

bool UDynamicInstanceSubsystem::ConvertInstanceToActor(FDynamicInstanceKey Key, FDynamicInstanceRecord& Record)
{
    if (Record.bIsConverted || !Key.IsValid() || !Key.SourceComponent.IsValid()) return false;
    
    UDynamicInstanceSourceComponent* SourceComp = Record.SourceComponent.Get();
    if (!SourceComp) return false;

    UDynamicConversionDefinition* Def = SourceComp->GetConversionDefinition();
    if (!Def || !Def->IsValidDefinition() || !Def->ActorClass)
    {
        DI_LOG(Error, TEXT("ConvertInstanceToActor failed: Invalid Definition on [%s]"), *SourceComp->GetOwner()->GetName());
        return false;
    }

    FTransform InstanceTransform;
    if (!SourceComp->GetInstanceWorldTransform(Key.SourceComponent.Get(), Key.InstanceIndex, InstanceTransform))
    {
        return false;
    }
    if (!SourceComp->HideInstance(Key.SourceComponent.Get(), Key.InstanceIndex))
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* NewActor = GetWorld()->SpawnActor<AActor>(Def->ActorClass, InstanceTransform, SpawnParams);

    if (NewActor)
    {
        Record.SpawnedActor = NewActor;
        Record.bIsConverted = true;
        Record.LastStateChangeTime = GetWorld()->GetTimeSeconds();
        Record.OriginalInstanceTransform = InstanceTransform;
        if (NewActor->GetClass()->ImplementsInterface(UDynamicConvertibleActor::StaticClass()))
        {
            IDynamicConvertibleActor::Execute_OnConvertedFromInstance(NewActor, InstanceTransform, Record.SavedGameplayState);
        }
        if (OnInstanceConverted.IsBound())
        {
        	OnInstanceConverted.Broadcast(Key.SourceComponent.Get(), Key.InstanceIndex, NewActor);
        }
        
        return true;
    }
    SourceComp->RestoreInstance(Key.SourceComponent.Get(), Key.InstanceIndex, InstanceTransform);
    return false;
}

bool UDynamicInstanceSubsystem::RevertActorToInstance(FDynamicInstanceKey Key, FDynamicInstanceRecord& Record)
{
	if (!Record.bIsConverted || !Record.SpawnedActor.IsValid()) return false;
	AActor* Actor = Record.SpawnedActor.Get();
	UDynamicInstanceSourceComponent* SourceComp = Record.SourceComponent.Get();
	if (!SourceComp) return false;
	if (Actor->GetClass()->ImplementsInterface(UDynamicConvertibleActor::StaticClass()))
	{
		if (!IDynamicConvertibleActor::Execute_CanRevertToInstance(Actor)) return false;
		IDynamicConvertibleActor::Execute_PrepareForReversion(Actor, Record.SavedGameplayState);
	}
	if (OnInstanceReverted.IsBound())
	{
		OnInstanceReverted.Broadcast(Key.SourceComponent.Get(), Key.InstanceIndex);
	}
	SourceComp->RestoreInstance(Key.SourceComponent.Get(), Key.InstanceIndex, Record.OriginalInstanceTransform);
	Actor->Destroy();
	Record.SpawnedActor = nullptr;
	Record.bIsConverted = false;
	Record.LastStateChangeTime = GetWorld()->GetTimeSeconds();

	return true;
}

bool UDynamicInstanceSubsystem::HasSatisfiedHysteresis(const FDynamicInstanceRecord& Record, const UDynamicConversionDefinition* Def) const
{
	if (!Def) return false;
	if (Record.LastStateChangeTime <= 0.0) return true;
	const double CurrentTime = GetWorld()->GetTimeSeconds();
	const double Elapsed = CurrentTime - Record.LastStateChangeTime;
	return Elapsed >= Def->HysteresisTime;
}

void UDynamicInstanceSubsystem::EvaluateSources()
{
	TArray<FVector> QueryLocations;
	GetCollectionOfQueryLocations(QueryLocations);
	if (QueryLocations.Num() == 0) return;
	if (bEnableDebugDraw)
	{
		for (const FVector& Loc : QueryLocations)
			DrawDebugSphere(GetWorld(), Loc, 50.0f, 12, FColor::Cyan, false, DebugDrawDuration, 0, 2.0f);
	}

	for (auto It = RegisteredSources.CreateIterator(); It; ++It)
	{
		UDynamicInstanceSourceComponent* Source = It->Get();
		if (!IsValid(Source) || !Source->HasValidDefinition()) continue;
		if (!IsSourceInBroadphaseRange(Source, QueryLocations)) continue;
		Source->ForEachISMComponent([&](UInstancedStaticMeshComponent* ISM)
		{
			const int32 Count = ISM->GetInstanceCount();
			for (int32 i = 0; i < Count; ++i)
			{
				ProcessInstance(Source, ISM, i, QueryLocations);
			}
		});
	}
}

void UDynamicInstanceSubsystem::GetCollectionOfQueryLocations(TArray<FVector>& OutLocations)
{
	FVector PlayerLoc;
	if (GetPrimaryQueryLocation(PlayerLoc))
	{
		OutLocations.Add(PlayerLoc);
	}

	for (auto It = RegisteredQueryActors.CreateIterator(); It; ++It)
	{
		if (AActor* Actor = It->Get())
		{
			OutLocations.Add(Actor->GetActorLocation());
		}
		else
		{
			It.RemoveCurrent();
		}
	}
}

bool UDynamicInstanceSubsystem::IsSourceInBroadphaseRange(const UDynamicInstanceSourceComponent* Source, const TArray<FVector>& QueryLocations) const
{
	AActor* Owner = Source->GetOwner();
	UDynamicConversionDefinition* Def = Source->GetConversionDefinition();
	if (!Owner || !Def) return false;
	const float RangeSq = FMath::Square(Def->RevertRadius + 5000.0f);
	const FVector OwnerLoc = Owner->GetActorLocation();
	for (const FVector& QLoc : QueryLocations)
	{
		if (FVector::DistSquared(QLoc, OwnerLoc) <= RangeSq) return true;
	}
	return false;
}

void UDynamicInstanceSubsystem::ProcessInstance(UDynamicInstanceSourceComponent* Source, UInstancedStaticMeshComponent* ISM, int32 Index, const TArray<FVector>& QueryLocations)
{
	FDynamicInstanceKey Key(ISM, Index);
	FDynamicInstanceRecord* Record = InstanceRegistry.Find(Key);
	UDynamicConversionDefinition* Def = Source->GetConversionDefinition();
	FVector InstanceLoc = (Record && Record->bIsConverted) 
		? Record->OriginalInstanceTransform.GetLocation() 
		: FVector::ZeroVector;

	if (!Record || !Record->bIsConverted)
	{
		FTransform TempXform;
		if (!Source->GetInstanceWorldTransform(ISM, Index, TempXform)) return;
		InstanceLoc = TempXform.GetLocation();
	}

	float MinDistSq = MAX_FLT;
	for (const FVector& QLoc : QueryLocations)
	{
		MinDistSq = FMath::Min(MinDistSq, FVector::DistSquared(QLoc, InstanceLoc));
	}

	if (bEnableDebugDraw) 
	{
		DrawDebugVisuals(InstanceLoc, MinDistSq, Def);
	}
	
	if (ShouldConvertInstance(Record, Def, MinDistSq))
	{
		if (bEnableDebugLogging) 
			DI_LOG(Log, TEXT("Converting Instance %d on %s"), Index, *Source->GetOwner()->GetName());
        
		ConvertInstance(Source, ISM, Index);
	}
	else if (ShouldRevertInstance(Record, Def, MinDistSq))
	{
		if (bEnableDebugLogging) 
			DI_LOG(Log, TEXT("Reverting Instance %d on %s"), Index, *Source->GetOwner()->GetName());

		RevertInstance(Key);
	}
}

void UDynamicInstanceSubsystem::DrawDebugVisuals(const FVector& InstanceLoc, float DistSq, const UDynamicConversionDefinition* Def) const
{
	if (!Def) return;
	DrawDebugPoint(GetWorld(), InstanceLoc, 5.0f, FColor(128, 128, 128), false, DebugDrawDuration);
	const float VisualizationRangeSq = FMath::Square(Def->RevertRadius * 1.5f);
	if (DistSq < VisualizationRangeSq)
	{
		DrawDebugCircle(GetWorld(), InstanceLoc, Def->ConversionRadius, 32, FColor::Green, false, DebugDrawDuration, 0, 1.0f, FVector(0, 0, 1));
		DrawDebugCircle(GetWorld(), InstanceLoc, Def->RevertRadius, 32, FColor::Red, false, DebugDrawDuration, 0, 1.0f, FVector(0, 0, 1));
	}
}

void UDynamicInstanceSubsystem::RegisterQueryActor(AActor* QueryActor)
{
	if (IsValid(QueryActor))
	{
		RegisteredQueryActors.Add(QueryActor);
		DI_LOG(Log, TEXT("Registered Query Actor: %s. Total Query Actors: %d"), 
			*QueryActor->GetName(), RegisteredQueryActors.Num());
	}
}

void UDynamicInstanceSubsystem::UnregisterQueryActor(AActor* QueryActor)
{
	if (RegisteredQueryActors.Contains(QueryActor))
	{
		RegisteredQueryActors.Remove(QueryActor);
		DI_LOG(Log, TEXT("Unregistered Query Actor: %s"), *QueryActor->GetName());
	}
}

bool UDynamicInstanceSubsystem::ConvertInstance(UDynamicInstanceSourceComponent* Source, UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	FDynamicInstanceKey Key(ISM, InstanceIndex);
	FDynamicInstanceRecord& Record = InstanceRegistry.FindOrAdd(Key);
	Record.Key = Key;
	Record.SourceComponent = Source; 
	return ConvertInstanceToActor(Key, Record);
}

bool UDynamicInstanceSubsystem::RevertInstance(const FDynamicInstanceKey& Key)
{
	FDynamicInstanceRecord* Record = InstanceRegistry.Find(Key);
	if (Record && Record->bIsConverted)
	{
		return RevertActorToInstance(Key, *Record);
	}
	return false;
}

bool UDynamicInstanceSubsystem::GetPrimaryQueryLocation(FVector& OutLocation) const
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			OutLocation = Pawn->GetActorLocation();
			return true;
		}
	}
	return false;
}

void UDynamicInstanceSubsystem::PruneStaleRecords()
{
	int32 InitialCount = InstanceRegistry.Num();
	for (auto It = InstanceRegistry.CreateIterator(); It; ++It)
	{
		const FDynamicInstanceRecord& Record = It.Value();
		if (Record.bIsConverted || Record.SavedGameplayState.IsValid())
		{
			continue;
		}
		It.RemoveCurrent();
	}
	int32 FinalCount = InstanceRegistry.Num();
	if (InitialCount != FinalCount)
	{
		DI_LOG(Log, TEXT("Registry Pruned: Removed %d stale records. Current size: %d"), 
			(InitialCount - FinalCount), FinalCount);
	}
}

bool UDynamicInstanceSubsystem::ShouldConvertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const
{
	if (!Def)
	{
		return false;
	}

	if (DistSq > FMath::Square(Def->ConversionRadius))
	{
		return false;
	}

	if (Record && Record->bIsConverted)
	{
		return false;
	}

	if (Record && !HasSatisfiedHysteresis(*Record, Def))
	{
		return false;
	}

	return true;
}

bool UDynamicInstanceSubsystem::ShouldRevertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const
{
	if (!Def || !Record)
	{
		return false;
	}

	if (!Record->bIsConverted)
	{
		return false;
	}

	if (DistSq <= FMath::Square(Def->RevertRadius))
	{
		return false;
	}

	if (!HasSatisfiedHysteresis(*Record, Def))
	{
		return false;
	}

	return true;
}

bool UDynamicInstanceSubsystem::IsInstanceConverted(const FDynamicInstanceKey& Key) const
{
	const FDynamicInstanceRecord* Record = InstanceRegistry.Find(Key);
	return Record && Record->bIsConverted;
}

AActor* UDynamicInstanceSubsystem::GetActorForKey(const FDynamicInstanceKey& Key) const
{
	if (const FDynamicInstanceRecord* Record = InstanceRegistry.Find(Key))
	{
		return Record->SpawnedActor.Get();
	}
	return nullptr;
}

int32 UDynamicInstanceSubsystem::GetRegistrySize() const
{
	return InstanceRegistry.Num();
}

bool UDynamicInstanceSubsystem::ManualConvert(UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	if (!IsValid(ISM)) return false;
	if (!ISM->IsValidInstance(InstanceIndex))
	{
		DI_LOG(Warning, TEXT("ManualConvert failed: Instance index %d is out of bounds for ISM [%s] on Actor [%s]"), 
			InstanceIndex, *ISM->GetName(), *ISM->GetOwner()->GetName());
		return false;
	}
	
	AActor* Owner = ISM->GetOwner();
	if (!Owner)
	{
		return false;
	}
	
	UDynamicInstanceSourceComponent* Source = ISM->GetOwner()->FindComponentByClass<UDynamicInstanceSourceComponent>();
	if (!Source)
	{
		DI_LOG(Warning, TEXT("ManualConvert failed: Actor [%s] is missing a UDynamicInstanceSourceComponent"), 
			*ISM->GetOwner()->GetName());
		return false;
	}

	return ConvertInstance(Source, ISM, InstanceIndex);
}

bool UDynamicInstanceSubsystem::ManualRevert(UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	if (!IsValid(ISM)) return false;
	if (!ISM->IsValidInstance(InstanceIndex))
	{
		return false;
	}
	const FDynamicInstanceKey Key(ISM, InstanceIndex);
	return RevertInstance(Key);
}