
#include "Library/DynamicInstanceTestingLibrary.h"
#include "Core/DynamicInstanceSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Library/DynamicInstanceLibrary.h"

void UDynamicInstanceTestingLibrary::SetupDefaultTestingEnvironment(const UObject* WorldContextObject, bool bEnableDebug, float UpdateFrequency)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return;

	UDynamicInstanceSubsystem* Subsystem = World->GetSubsystem<UDynamicInstanceSubsystem>();
	if (!Subsystem) return;
	Subsystem->bEnableDebugDraw = bEnableDebug;
	Subsystem->UpdateInterval = UpdateFrequency;

	// 2. Auto-Register the local player pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (PlayerPawn)
	{
		Subsystem->RegisterQueryActor(PlayerPawn);
	}

	UE_LOG(LogTemp, Log, TEXT("DIS Testing: Environment Setup Complete. Player Registered."));
}

void UDynamicInstanceTestingLibrary::RunSystemDiagnostic(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return;

	if (UDynamicInstanceSubsystem* Subsystem = World->GetSubsystem<UDynamicInstanceSubsystem>())
	{
		int32 Count = Subsystem->GetRegistrySize();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, 
			FString::Printf(TEXT("DIS Diagnostic: %d Records in Registry"), Count));
	}
}

void UDynamicInstanceTestingLibrary::ForceConvertLookAtTarget(const UObject* WorldContextObject, float MaxDistance)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC) return;

	FVector Start, Dir;
	PC->DeprojectMousePositionToWorld(Start, Dir);
    
	FHitResult Hit;
	FVector End = Start + (Dir * MaxDistance);
    
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
	{
		if (UInstancedStaticMeshComponent* ISM = Cast<UInstancedStaticMeshComponent>(Hit.Component))
		{
			UDynamicInstanceLibrary::ForceConvertInstance(WorldContextObject, ISM, Hit.Item);
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("Forced Conversion on Index: %d"), Hit.Item));
		}
	}
}