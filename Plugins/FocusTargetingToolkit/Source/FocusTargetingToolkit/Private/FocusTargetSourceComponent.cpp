#include "FocusTargetSourceComponent.h"

#include "FocusTargetingProfile.h"
#include "FocusTargetingSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UFocusTargetSourceComponent::UFocusTargetSourceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UFocusTargetSourceComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bAutoUpdateFocus)
    {
        return;
    }

    TimeSinceLastTrace += DeltaTime;

    if (TraceInterval > 0.0f && TimeSinceLastTrace < TraceInterval)
    {
        return;
    }

    TimeSinceLastTrace = 0.0f;
    UpdateFocusNow();
}

bool UFocusTargetSourceComponent::UpdateFocusNow()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    UFocusTargetingSubsystem* Subsystem = World->GetSubsystem<UFocusTargetingSubsystem>();
    if (!Subsystem)
    {
        return false;
    }

    FVector Start = FVector::ZeroVector;
    FVector Direction = FVector::ForwardVector;
    AActor* Instigator = nullptr;

    if (!ResolveTraceView(Start, Direction, Instigator))
    {
        return false;
    }

    return Subsystem->UpdateSoftFocusFromRaycast(
        Start,
        Direction,
        OverrideProfile,
        Instigator);
}

bool UFocusTargetSourceComponent::ResolveTraceView(
    FVector& OutStart,
    FVector& OutDirection,
    AActor*& OutInstigator) const
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    OutInstigator = Owner;

    APawn* OwnerPawn = Cast<APawn>(Owner);
    APlayerController* PC = nullptr;

    if (OwnerPawn)
    {
        PC = Cast<APlayerController>(OwnerPawn->GetController());

        if (bOnlyLocalPlayer && !OwnerPawn->IsLocallyControlled())
        {
            return false;
        }
    }
    else
    {
        PC = Cast<APlayerController>(Owner);

        if (PC)
        {
            OutInstigator = PC->GetPawn();

            if (bOnlyLocalPlayer && !PC->IsLocalController())
            {
                return false;
            }
        }
    }

    if (SourceMode == EFocusTargetSourceMode::PlayerCamera && PC && PC->PlayerCameraManager)
    {
        OutStart = PC->PlayerCameraManager->GetCameraLocation();
        OutDirection = PC->PlayerCameraManager->GetCameraRotation().Vector();
        return true;
    }

    OutStart = Owner->GetActorLocation();
    OutDirection = Owner->GetActorForwardVector();
    return true;
}