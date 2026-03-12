#include "Core/DynamicInstanceSubsystem.h"
#include "Components/DynamicInstanceSourceComponent.h"
#include "Data/DynamicConversionDefinition.h"
#include "Interfaces/IDynamicConvertibleActor.h"
#include "GameFramework/Pawn.h"
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
	if (GetPrimaryQueryLocation(QueryLoc))
	{
		EvaluateSources();
	}
}

bool UDynamicInstanceSubsystem::ConvertInstanceToActor(FDynamicInstanceKey Key, FDynamicInstanceRecord& Record)
{
	if (Record.bIsConverted || !Key.IsValid() || !Key.SourceComponent.IsValid()) return false;
	UDynamicInstanceSourceComponent* SourceComp = Record.SourceComponent.Get();
	if (!SourceComp) return false;

	UDynamicConversionDefinition* Def = SourceComp->GetConversionDefinition();
	if (!Def || !Def->IsValidDefinition() || !Def->ActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DynamicInstanceSystem: Attempted conversion with invalid Definition or ActorClass."));
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
		Record.CachedInstanceTransform = InstanceTransform;

		if (NewActor->GetClass()->ImplementsInterface(UDynamicConvertibleActor::StaticClass()))
		{
			IDynamicConvertibleActor::Execute_OnConvertedFromInstance(NewActor, InstanceTransform, Record.SavedGameplayState);
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
	SourceComp->RestoreInstance(Key.SourceComponent.Get(), Key.InstanceIndex, Record.CachedInstanceTransform);
	Actor->Destroy();
	Record.SpawnedActor = nullptr;
	Record.bIsConverted = false;
	Record.LastStateChangeTime = GetWorld()->GetTimeSeconds();
	return true;
}

bool UDynamicInstanceSubsystem::HasSatisfiedHysteresis(const FDynamicInstanceRecord& Record, const UDynamicConversionDefinition* Def) const
{
    if (!Def) return false;

    const double CurrentTime = GetWorld()->GetTimeSeconds();
    const double Elapsed = CurrentTime - Record.LastStateChangeTime;
    
    return Elapsed >= Def->HysteresisTime;
}

void UDynamicInstanceSubsystem::EvaluateSources()
{
    FVector QueryLoc;
    if (!GetPrimaryQueryLocation(QueryLoc)) return;
    for (auto It = RegisteredSources.CreateIterator(); It; ++It)
    {
        UDynamicInstanceSourceComponent* Source = It->Get();
        if (!IsValid(Source) || !Source->HasValidDefinition()) continue;
        
        AActor* Owner = Source->GetOwner();
        UDynamicConversionDefinition* Def = Source->GetConversionDefinition();
        const float BroadRangeSq = FMath::Square(Def->RevertRadius + 5000.0f);
        if (FVector::DistSquared(QueryLoc, Owner->GetActorLocation()) > BroadRangeSq)
        {
            continue;
        }

        Source->ForEachISMComponent([&](UInstancedStaticMeshComponent* ISM)
        {
            const int32 InstanceCount = ISM->GetInstanceCount();
            for (int32 i = 0; i < InstanceCount; ++i)
            {
                FDynamicInstanceKey Key(ISM, i);
                FDynamicInstanceRecord* Record = InstanceRegistry.Find(Key);
                FVector InstanceLoc = FVector::ZeroVector;
                if (Record && Record->bIsConverted)
                {
                    InstanceLoc = Record->CachedInstanceTransform.GetLocation();
                }
                else
                {
                    FTransform TempXform;
                    if (Source->GetInstanceWorldTransform(ISM, i, TempXform))
                    {
                        InstanceLoc = TempXform.GetLocation();
                    }
                    else continue;
                }
                const float DistSq = FVector::DistSquared(QueryLoc, InstanceLoc);
                if (ShouldConvertInstance(Record, Def, DistSq))
                {
                    ConvertInstance(Source, ISM, i);
                }
                else if (ShouldRevertInstance(Record, Def, DistSq))
                {
                    RevertInstance(Key);
                }
            }
        });
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

bool UDynamicInstanceSubsystem::GetPlayerLocation(FVector& OutLocation) const
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
	for (auto It = InstanceRegistry.CreateIterator(); It; ++It)
	{
		const FDynamicInstanceRecord& Record = It.Value();
		if (!Record.bIsConverted && !Record.SavedGameplayState.IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

bool UDynamicInstanceSubsystem::ShouldConvertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const
{
	if (!Def) return false;
	if (DistSq > FMath::Square(Def->ConversionRadius)) return false;
	if (Record && Record->bIsConverted) return false;
	if (Record)
	{
		const double CurrentTime = GetWorld()->GetTimeSeconds();
		if ((CurrentTime - Record->LastStateChangeTime) < Def->HysteresisTime)
		{
			return false;
		}
	}

	return true;
}

bool UDynamicInstanceSubsystem::ShouldRevertInstance(const FDynamicInstanceRecord* Record, const UDynamicConversionDefinition* Def, float DistSq) const
{
	if (!Def || !Record) return false;
	if (!Record->bIsConverted) return false;
	if (DistSq <= FMath::Square(Def->RevertRadius)) return false;
	const double CurrentTime = GetWorld()->GetTimeSeconds();
	if ((CurrentTime - Record->LastStateChangeTime) < Def->HysteresisTime)
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
	UDynamicInstanceSourceComponent* Source = ISM->GetOwner()->FindComponentByClass<UDynamicInstanceSourceComponent>();
	if (!Source) return false;
	return ConvertInstance(Source, ISM, InstanceIndex);
}

bool UDynamicInstanceSubsystem::ManualRevert(UInstancedStaticMeshComponent* ISM, int32 InstanceIndex)
{
	FDynamicInstanceKey Key(ISM, InstanceIndex);
	return RevertInstance(Key);
}
