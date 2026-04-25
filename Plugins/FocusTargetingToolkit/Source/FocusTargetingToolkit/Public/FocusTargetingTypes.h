// FocusTargetingTypes.h
#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "FocusTargetingTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EFocusLockState : uint8
{
    None        UMETA(DisplayName="None"),
    SoftLocked  UMETA(DisplayName="Soft Locked"),
    HardLocked  UMETA(DisplayName="Hard Locked")
};

UENUM(BlueprintType)
enum class EFocusHitType : uint8
{
    None       UMETA(DisplayName="None"),
    Invalid    UMETA(DisplayName="Invalid"),
    Actor      UMETA(DisplayName="Actor"),
    Component  UMETA(DisplayName="Component"),
    Instance   UMETA(DisplayName="Instance")
};

USTRUCT(BlueprintType)
struct FFocusHandle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    TWeakObjectPtr<AActor> TargetActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    TWeakObjectPtr<UPrimitiveComponent> TargetComponent = nullptr;

    /** For InstancedStaticMeshComponent-style targets. INDEX_NONE means whole component/actor. */
    UPROPERTY(BlueprintReadOnly, Category="Focus")
    int32 ElementIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    FName SocketName = NAME_None;

    FFocusHandle() = default;

    explicit FFocusHandle(AActor* InActor)
        : TargetActor(InActor)
    {
    }

    FFocusHandle(AActor* InActor, UPrimitiveComponent* InComponent, int32 InElementIndex = INDEX_NONE)
        : TargetActor(InActor)
        , TargetComponent(InComponent)
        , ElementIndex(InElementIndex)
    {
    }

    bool IsValid() const
    {
        return TargetActor.IsValid() || TargetComponent.IsValid();
    }

    AActor* GetActor() const
    {
        if (TargetActor.IsValid())
        {
            return TargetActor.Get();
        }

        return TargetComponent.IsValid() ? TargetComponent->GetOwner() : nullptr;
    }

    UPrimitiveComponent* GetComponent() const
    {
        return TargetComponent.Get();
    }

    void Reset()
    {
        TargetActor = nullptr;
        TargetComponent = nullptr;
        ElementIndex = INDEX_NONE;
        SocketName = NAME_None;
    }

    bool operator==(const FFocusHandle& Other) const
    {
        return TargetActor == Other.TargetActor
            && TargetComponent == Other.TargetComponent
            && ElementIndex == Other.ElementIndex
            && SocketName == Other.SocketName;
    }

    bool operator!=(const FFocusHandle& Other) const
    {
        return !(*this == Other);
    }
};

USTRUCT(BlueprintType)
struct FFocusResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    bool bHadBlockingHit = false;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    bool bIsValidFocusTarget = false;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    EFocusLockState LockState = EFocusLockState::None;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    EFocusHitType HitType = EFocusHitType::None;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    FFocusHandle Handle;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    TWeakObjectPtr<AActor> HitActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    TWeakObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    FVector ImpactPoint = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="Focus")
    FVector ImpactNormal = FVector::UpVector;

    /** Unreal units / centimeters. */
    UPROPERTY(BlueprintReadOnly, Category="Focus")
    float Distance = 0.0f;

    void Reset()
    {
        bHadBlockingHit = false;
        bIsValidFocusTarget = false;
        LockState = EFocusLockState::None;
        HitType = EFocusHitType::None;
        Handle.Reset();
        HitActor = nullptr;
        HitComponent = nullptr;
        ImpactPoint = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        Distance = 0.0f;
    }

    bool HasUsableTarget() const
    {
        return bHadBlockingHit && bIsValidFocusTarget && Handle.IsValid();
    }

    bool HasSameTargetIdentity(const FFocusResult& Other) const
    {
        return bHadBlockingHit == Other.bHadBlockingHit
            && bIsValidFocusTarget == Other.bIsValidFocusTarget
            && LockState == Other.LockState
            && HitType == Other.HitType
            && Handle == Other.Handle;
    }
};