#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FocusTargetingTypes.h"
#include "FocusTargetingLibrary.generated.h"

class UFocusTargetingProfile;

UCLASS()
class FOCUSTARGETINGTOOLKIT_API UFocusTargetingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Focus", meta=(WorldContext="WorldContextObject"))
	static bool UpdateFocusFromCamera(
		const UObject* WorldContextObject,
		APlayerController* PlayerController,
		UFocusTargetingProfile* Profile = nullptr);

	UFUNCTION(BlueprintCallable, Category="Focus", meta=(WorldContext="WorldContextObject"))
	static bool PromoteSoftFocusToHardFocus(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Focus", meta=(WorldContext="WorldContextObject"))
	static void ClearFocus(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Focus", meta=(WorldContext="WorldContextObject"))
	static FFocusResult GetCurrentFocus(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Focus", meta=(WorldContext="WorldContextObject"))
	static bool HasHardFocus(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category="Focus", meta=(WorldContext="WorldContextObject"))
	static bool HasSoftFocus(const UObject* WorldContextObject);
};