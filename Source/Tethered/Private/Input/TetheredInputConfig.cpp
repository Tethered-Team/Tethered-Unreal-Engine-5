// Copyright Nicholas Reardon

#include "Input/TetheredInputConfig.h"


const UInputAction *UTetheredInputConfig::FindAbilityInputActionForTag(const FGameplayTag &InputTag,
																	   bool bLogNotFound) const
{
	for (const FTetheredInputAction &Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for tag [%s], on InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
