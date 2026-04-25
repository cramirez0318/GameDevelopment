#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "FocusTargetingTypes.h"
#include "FocusTargetComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup=(Focus), meta=(BlueprintSpawnableComponent))
class FOCUSTARGETINGTOOLKIT_API UFocusTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFocusTargetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Identity")
	FGameplayTagContainer FocusTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Capabilities")
	bool bCanSoftLock = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Capabilities")
	bool bCanHardLock = true;

	/** Optional component this focus target represents. If unset, the owner actor is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Target")
	TWeakObjectPtr<UPrimitiveComponent> TargetComponentOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Target")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Visuals")
	FGameplayTag SoftLockVisualTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus|Visuals")
	FGameplayTag HardLockVisualTag;

	UFUNCTION(BlueprintCallable, Category="Focus")
	FFocusHandle GetFocusHandle(int32 ElementIndex = -1) const;

	UFUNCTION(BlueprintPure, Category="Focus")
	bool CanSoftLock() const { return bCanSoftLock; }

	UFUNCTION(BlueprintPure, Category="Focus")
	bool CanHardLock() const { return bCanHardLock; }

	UFUNCTION(BlueprintPure, Category="Focus")
	FText GetResolvedDisplayName() const;
};