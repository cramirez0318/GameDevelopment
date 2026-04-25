#include "FocusTargetingLibrary.h"

#include "FocusTargetingProfile.h"
#include "FocusTargetingSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

namespace
{
    UFocusTargetingSubsystem* GetFocusSubsystem(const UObject* WorldContextObject)
    {
        if (!GEngine)
        {
            return nullptr;
        }

        UWorld* World = GEngine->GetWorldFromContextObject(
            WorldContextObject,
            EGetWorldErrorMode::LogAndReturnNull);

        return World ? World->GetSubsystem<UFocusTargetingSubsystem>() : nullptr;
    }
}

bool UFocusTargetingLibrary::UpdateFocusFromCamera(
    const UObject* WorldContextObject,
    APlayerController* PlayerController,
    UFocusTargetingProfile* Profile)
{
    if (!PlayerController || !PlayerController->PlayerCameraManager)
    {
        return false;
    }

    UFocusTargetingSubsystem* Subsystem = GetFocusSubsystem(WorldContextObject);
    if (!Subsystem)
    {
        return false;
    }

    const FVector Start = PlayerController->PlayerCameraManager->GetCameraLocation();
    const FVector Direction = PlayerController->PlayerCameraManager->GetCameraRotation().Vector();

    return Subsystem->UpdateSoftFocusFromRaycast(
        Start,
        Direction,
        Profile,
        PlayerController->GetPawn());
}

bool UFocusTargetingLibrary::PromoteSoftFocusToHardFocus(const UObject* WorldContextObject)
{
    if (UFocusTargetingSubsystem* Subsystem = GetFocusSubsystem(WorldContextObject))
    {
        return Subsystem->PromoteSoftLockToHardLock();
    }

    return false;
}

void UFocusTargetingLibrary::ClearFocus(const UObject* WorldContextObject)
{
    if (UFocusTargetingSubsystem* Subsystem = GetFocusSubsystem(WorldContextObject))
    {
        Subsystem->ClearFocus();
    }
}

FFocusResult UFocusTargetingLibrary::GetCurrentFocus(const UObject* WorldContextObject)
{
    if (UFocusTargetingSubsystem* Subsystem = GetFocusSubsystem(WorldContextObject))
    {
        return Subsystem->GetCurrentFocusResult();
    }

    return FFocusResult{};
}

bool UFocusTargetingLibrary::HasHardFocus(const UObject* WorldContextObject)
{
    if (UFocusTargetingSubsystem* Subsystem = GetFocusSubsystem(WorldContextObject))
    {
        return Subsystem->HasHardFocus();
    }

    return false;
}

bool UFocusTargetingLibrary::HasSoftFocus(const UObject* WorldContextObject)
{
    if (UFocusTargetingSubsystem* Subsystem = GetFocusSubsystem(WorldContextObject))
    {
        return Subsystem->HasSoftFocus();
    }

    return false;
}