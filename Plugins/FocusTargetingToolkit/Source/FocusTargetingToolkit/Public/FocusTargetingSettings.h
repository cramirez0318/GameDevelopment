#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "FocusTargetingSettings.generated.h"

class UFocusTargetingProfile;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Focus Targeting Toolkit"))
class FOCUSTARGETINGTOOLKIT_API UFocusTargetingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFocusTargetingSettings();

	UPROPERTY(Config, EditAnywhere, Category="General")
	TSoftObjectPtr<UFocusTargetingProfile> DefaultProfile;

	UPROPERTY(Config, EditAnywhere, Category="General")
	bool bUseDefaultProfileWhenNoneProvided = true;

	static const UFocusTargetingSettings* Get()
	{
		return GetDefault<UFocusTargetingSettings>();
	}
};