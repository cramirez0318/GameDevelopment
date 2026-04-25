#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FocusTargetSourceComponent.generated.h"

class UFocusTargetingProfile;

UENUM(BlueprintType)
enum class EFocusTargetSourceMode : uint8
{
    PlayerCamera    UMETA(DisplayName="Player Camera"),
    ActorForward    UMETA(DisplayName="Actor Forward")
};

/**
 * Add this to a Character, Pawn, or PlayerController to automatically drive
 * the FocusTargetingSubsystem.
 */
UCLASS(ClassGroup=(Focus), meta=(BlueprintSpawnableComponent))
class FOCUSTARGETINGTOOLKIT_API UFocusTargetSourceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFocusTargetSourceComponent();

    /** Optional override. If null, uses the Project Settings default profile. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
    TObjectPtr<UFocusTargetingProfile> OverrideProfile = nullptr;

    /** How often to trace. 0.0 = every frame. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus", meta=(ClampMin="0.0"))
    float TraceInterval = 0.0f;

    /** Camera-based tracing is best for player targeting. ActorForward is useful for AI or simple pawns. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
    EFocusTargetSourceMode SourceMode = EFocusTargetSourceMode::PlayerCamera;

    /** If true, only a locally controlled pawn/controller will drive focus. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
    bool bOnlyLocalPlayer = true;

    /** If false, this component will not update focus. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
    bool bAutoUpdateFocus = true;

    UFUNCTION(BlueprintCallable, Category="Focus")
    bool UpdateFocusNow();

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TimeSinceLastTrace = 0.0f;

    bool ResolveTraceView(FVector& OutStart, FVector& OutDirection, AActor*& OutInstigator) const;
};