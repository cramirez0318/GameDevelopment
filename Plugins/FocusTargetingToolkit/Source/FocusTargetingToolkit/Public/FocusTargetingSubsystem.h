#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FocusTargetingTypes.h"
#include "FocusTargetingSubsystem.generated.h"

class UFocusTargetingProfile;
class UFocusTargetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFocusResultChangedSignature, const FFocusResult&, NewResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFocusLockChangedSignature, const FFocusResult&, NewResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFocusClearedSignature);

UCLASS()
class FOCUSTARGETINGTOOLKIT_API UFocusTargetingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Focus")
    bool UpdateSoftFocusFromRaycast(
        const FVector& Start,
        const FVector& Direction,
        const UFocusTargetingProfile* Profile,
        AActor* Instigator = nullptr);

    UFUNCTION(BlueprintCallable, Category="Focus")
    bool PromoteSoftLockToHardLock();

    UFUNCTION(BlueprintCallable, Category="Focus")
    bool SetHardFocusFromActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category="Focus")
    void ClearFocus();

    UFUNCTION(BlueprintPure, Category="Focus")
    FFocusResult GetCurrentFocusResult() const { return CurrentResult; }

    UFUNCTION(BlueprintPure, Category="Focus")
    FFocusResult GetSoftFocusResult() const { return SoftResult; }

    UFUNCTION(BlueprintPure, Category="Focus")
    FFocusResult GetHardFocusResult() const { return HardResult; }

    UFUNCTION(BlueprintPure, Category="Focus")
    bool HasSoftFocus() const { return SoftResult.LockState == EFocusLockState::SoftLocked && SoftResult.HasUsableTarget(); }

    UFUNCTION(BlueprintPure, Category="Focus")
    bool HasHardFocus() const { return HardResult.LockState == EFocusLockState::HardLocked && HardResult.HasUsableTarget(); }

    UPROPERTY(BlueprintAssignable, Category="Focus|Events")
    FFocusResultChangedSignature OnFocusResultChanged;

    UPROPERTY(BlueprintAssignable, Category="Focus|Events")
    FFocusLockChangedSignature OnSoftLockChanged;

    UPROPERTY(BlueprintAssignable, Category="Focus|Events")
    FFocusLockChangedSignature OnHardLockChanged;

    UPROPERTY(BlueprintAssignable, Category="Focus|Events")
    FFocusClearedSignature OnFocusCleared;

private:
    UPROPERTY(Transient)
    FFocusResult CurrentResult;

    UPROPERTY(Transient)
    FFocusResult SoftResult;

    UPROPERTY(Transient)
    FFocusResult HardResult;

    bool BuildFocusResultFromHit(
        const FHitResult& Hit,
        const UFocusTargetingProfile* Profile,
        EFocusLockState DesiredLockState,
        FFocusResult& OutResult) const;

    UFocusTargetComponent* ResolveFocusComponentFromHit(const FHitResult& Hit) const;
    bool IsValidTarget(const UFocusTargetComponent* TargetComp, const UFocusTargetingProfile* Profile, EFocusLockState DesiredLockState) const;
    void SetCurrentResult(const FFocusResult& NewResult);
    const UFocusTargetingProfile* ResolveProfile(const UFocusTargetingProfile* Profile) const;
};