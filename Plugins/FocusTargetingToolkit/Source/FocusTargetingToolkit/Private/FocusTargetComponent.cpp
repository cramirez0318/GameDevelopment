#include "FocusTargetComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

UFocusTargetComponent::UFocusTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DisplayName = FText::FromString(TEXT("Focus Target"));
}

FFocusHandle UFocusTargetComponent::GetFocusHandle(int32 ElementIndex) const
{
	AActor* Owner = GetOwner();

	FFocusHandle Handle(
		Owner,
		TargetComponentOverride.Get(),
		ElementIndex
	);

	Handle.SocketName = SocketName;
	return Handle;
}

FText UFocusTargetComponent::GetResolvedDisplayName() const
{
	if (!DisplayName.IsEmpty())
	{
		return DisplayName;
	}

	if (const AActor* Owner = GetOwner())
	{
		return FText::FromString(Owner->GetActorNameOrLabel());
	}

	return FText::FromString(TEXT("Focus Target"));
}