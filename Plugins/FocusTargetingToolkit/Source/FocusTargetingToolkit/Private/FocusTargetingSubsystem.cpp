#include "FocusTargetingSubsystem.h"

#include "FocusTargetComponent.h"
#include "FocusTargetingProfile.h"
#include "FocusTargetingSettings.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

bool UFocusTargetingSubsystem::UpdateSoftFocusFromRaycast(
    const FVector& Start,
    const FVector& Direction,
    const UFocusTargetingProfile* Profile,
    AActor* Instigator)
{
    const UFocusTargetingProfile* ActiveProfile = ResolveProfile(Profile);

    if (!ActiveProfile || !GetWorld() || !ActiveProfile->bAllowSoftLock)
    {
        return false;
    }
    
    if (HasHardFocus())
    {
        return true;
    }

    const FVector SafeDirection = Direction.GetSafeNormal();
    if (SafeDirection.IsNearlyZero())
    {
        return false;
    }

    const FVector End = Start + (SafeDirection * Profile->MaxTraceDistance);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(FocusTargetingTrace), Profile->bTraceComplex);
    if (Profile->bIgnoreInstigator && Instigator)
    {
        Params.AddIgnoredActor(Instigator);
    }

    FHitResult Hit;
    bool bHit = false;

    if (Profile->bUseSphereTrace)
    {
        bHit = GetWorld()->SweepSingleByChannel(
            Hit,
            Start,
            End,
            FQuat::Identity,
            Profile->TraceChannel,
            FCollisionShape::MakeSphere(Profile->SphereRadius),
            Params);
    }
    else
    {
        bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,
            Start,
            End,
            Profile->TraceChannel,
            Params);
    }

#if !UE_BUILD_SHIPPING
    if (Profile->bDrawDebugTrace)
    {
        const FVector DebugEnd = bHit ? Hit.ImpactPoint : End;
        const FColor DrawColor = bHit ? Profile->DebugHitColor : Profile->DebugTraceColor;

        DrawDebugLine(GetWorld(), Start, DebugEnd, DrawColor, false, Profile->DebugDrawDuration, 0, 1.0f);

        if (Profile->bUseSphereTrace)
        {
            DrawDebugSphere(GetWorld(), DebugEnd, Profile->SphereRadius, 12, DrawColor, false, Profile->DebugDrawDuration);
        }
    }
#endif

    FFocusResult NewResult;
    NewResult.bHadBlockingHit = bHit;

    if (bHit)
    {
        BuildFocusResultFromHit(Hit, Profile, EFocusLockState::SoftLocked, NewResult);
    }

    SoftResult = NewResult;

    if (!NewResult.HasSameTargetIdentity(CurrentResult))
    {
        SetCurrentResult(NewResult);

        if (NewResult.HasUsableTarget())
        {
            OnSoftLockChanged.Broadcast(NewResult);
        }
        else
        {
            OnFocusCleared.Broadcast();
        }
    }

    return NewResult.HasUsableTarget();
}

bool UFocusTargetingSubsystem::PromoteSoftLockToHardLock()
{
    if (!HasSoftFocus())
    {
        return false;
    }

    HardResult = SoftResult;
    HardResult.LockState = EFocusLockState::HardLocked;

    SoftResult.Reset();

    SetCurrentResult(HardResult);
    OnHardLockChanged.Broadcast(HardResult);

    return true;
}

bool UFocusTargetingSubsystem::SetHardFocusFromActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return false;
    }

    UFocusTargetComponent* FocusComponent = TargetActor->FindComponentByClass<UFocusTargetComponent>();
    if (!FocusComponent || !FocusComponent->CanHardLock())
    {
        return false;
    }

    FFocusResult NewResult;
    NewResult.bHadBlockingHit = false;
    NewResult.bIsValidFocusTarget = true;
    NewResult.LockState = EFocusLockState::HardLocked;
    NewResult.HitType = EFocusHitType::Actor;
    NewResult.HitActor = TargetActor;
    NewResult.Handle = FocusComponent->GetFocusHandle();

    HardResult = NewResult;
    SoftResult.Reset();

    SetCurrentResult(HardResult);
    OnHardLockChanged.Broadcast(HardResult);

    return true;
}

void UFocusTargetingSubsystem::ClearFocus()
{
    const bool bHadFocus = CurrentResult.HasUsableTarget() || SoftResult.HasUsableTarget() || HardResult.HasUsableTarget();

    CurrentResult.Reset();
    SoftResult.Reset();
    HardResult.Reset();

    if (bHadFocus)
    {
        OnFocusCleared.Broadcast();
    }

    OnFocusResultChanged.Broadcast(CurrentResult);
}

bool UFocusTargetingSubsystem::BuildFocusResultFromHit(
    const FHitResult& Hit,
    const UFocusTargetingProfile* Profile,
    EFocusLockState DesiredLockState,
    FFocusResult& OutResult) const
{
    OutResult.Reset();

    OutResult.bHadBlockingHit = true;
    OutResult.HitActor = Hit.GetActor();
    OutResult.HitComponent = Hit.GetComponent();
    OutResult.ImpactPoint = Hit.ImpactPoint;
    OutResult.ImpactNormal = Hit.ImpactNormal;
    OutResult.Distance = Hit.Distance;

    UFocusTargetComponent* FocusComponent = ResolveFocusComponentFromHit(Hit);

    if (!IsValidTarget(FocusComponent, Profile, DesiredLockState))
    {
        OutResult.HitType = EFocusHitType::Invalid;
        return false;
    }

    OutResult.bIsValidFocusTarget = true;
    OutResult.LockState = DesiredLockState;

    if (FocusComponent)
    {
        OutResult.Handle = FocusComponent->GetFocusHandle(Hit.Item);
    }
    else
    {
        OutResult.Handle = FFocusHandle(Hit.GetActor(), Hit.GetComponent(), Hit.Item);
    }

    if (Hit.Item != INDEX_NONE)
    {
        OutResult.HitType = EFocusHitType::Instance;
    }
    else if (Hit.GetComponent())
    {
        OutResult.HitType = EFocusHitType::Component;
    }
    else if (Hit.GetActor())
    {
        OutResult.HitType = EFocusHitType::Actor;
    }
    else
    {
        OutResult.HitType = EFocusHitType::Invalid;
    }

    return OutResult.HasUsableTarget();
}

UFocusTargetComponent* UFocusTargetingSubsystem::ResolveFocusComponentFromHit(const FHitResult& Hit) const
{
    if (UPrimitiveComponent* HitComp = Hit.GetComponent())
    {
        if (AActor* Owner = HitComp->GetOwner())
        {
            TArray<UFocusTargetComponent*> Components;
            Owner->GetComponents<UFocusTargetComponent>(Components);

            for (UFocusTargetComponent* FocusComp : Components)
            {
                if (!FocusComp)
                {
                    continue;
                }

                if (FocusComp->TargetComponentOverride.IsValid() &&
                    FocusComp->TargetComponentOverride.Get() == HitComp)
                {
                    return FocusComp;
                }
            }

            if (Components.Num() > 0)
            {
                return Components[0];
            }
        }
    }

    if (AActor* HitActor = Hit.GetActor())
    {
        return HitActor->FindComponentByClass<UFocusTargetComponent>();
    }

    return nullptr;
}

bool UFocusTargetingSubsystem::IsValidTarget(
    const UFocusTargetComponent* TargetComp,
    const UFocusTargetingProfile* Profile,
    EFocusLockState DesiredLockState) const
{
    if (!Profile)
    {
        return false;
    }

    if (!TargetComp)
    {
        return !Profile->bRequireFocusComponent;
    }

    if (DesiredLockState == EFocusLockState::SoftLocked && !TargetComp->CanSoftLock())
    {
        return false;
    }

    if (DesiredLockState == EFocusLockState::HardLocked && !TargetComp->CanHardLock())
    {
        return false;
    }

    if (!Profile->RequiredFocusTags.IsEmpty() && !TargetComp->FocusTags.HasAny(Profile->RequiredFocusTags))
    {
        return false;
    }

    if (!Profile->BlockedFocusTags.IsEmpty() && TargetComp->FocusTags.HasAny(Profile->BlockedFocusTags))
    {
        return false;
    }

    return true;
}

void UFocusTargetingSubsystem::SetCurrentResult(const FFocusResult& NewResult)
{
    CurrentResult = NewResult;
    OnFocusResultChanged.Broadcast(CurrentResult);
}

const UFocusTargetingProfile* UFocusTargetingSubsystem::ResolveProfile(const UFocusTargetingProfile* Profile) const
{
    if (Profile)
    {
        return Profile;
    }

    const UFocusTargetingSettings* Settings = UFocusTargetingSettings::Get();
    if (!Settings || !Settings->bUseDefaultProfileWhenNoneProvided)
    {
        return nullptr;
    }

    return Settings->DefaultProfile.LoadSynchronous();
}