#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDynamicInstanceSystem, Log, All);

#define DI_LOG(Verbosity, Format, ...) UE_LOG(LogDynamicInstanceSystem, Verbosity, Format, ##__VA_ARGS__)

/**
 * Common Utility functions for the Dynamic Instance System
 */
class DYNAMICINSTANCESYSTEM_API FDynamicInstanceUtilities
{
public:
	// We can add static helper methods here later, 
	// like FName generators or Bounds calculators.
};