#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "FocusTargetingProfile.generated.h"

UCLASS(BlueprintType)
class FOCUSTARGETINGTOOLKIT_API UFocusTargetingProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UFocusTargetingProfile();

    // --- TRACE ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Trace", meta=(ClampMin="0.0"))
    float MaxTraceDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Trace")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Trace")
    bool bUseSphereTrace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Trace", meta=(EditCondition="bUseSphereTrace", ClampMin="0.0"))
    float SphereRadius = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Trace")
    bool bTraceComplex = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Trace")
    bool bIgnoreInstigator = true;

    // --- LOCKING ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Locking")
    bool bAllowSoftLock = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Locking")
    bool bAllowHardLock = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Locking")
    bool bClearSoftLockOnHardLock = true;

    // --- VALIDATION ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Validation")
    bool bRequireFocusComponent = true;

    /** If set, focused targets must have at least one of these tags. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Validation")
    FGameplayTagContainer RequiredFocusTags;

    /** If set, focused targets with any of these tags are rejected. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Validation")
    FGameplayTagContainer BlockedFocusTags;

    // --- DEBUG ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Debug")
    bool bDrawDebugTrace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Debug", meta=(EditCondition="bDrawDebugTrace"))
    FColor DebugTraceColor = FColor::Cyan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Debug", meta=(EditCondition="bDrawDebugTrace"))
    FColor DebugHitColor = FColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Debug", meta=(EditCondition="bDrawDebugTrace", ClampMin="0.0"))
    float DebugDrawDuration = 0.1f;
};