// Copyright Nicholas Reardon


#include "AbilitySystem/TetheredAbilitySystemComponent.h"

#include "TetheredGameplayTags.h"
#include "AbilitySystem/Abilities/TetheredGameplayAbility.h"
#include "Tethered.h"

void UTetheredAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UTetheredAbilitySystemComponent::ClientEffectApplied);


}

void UTetheredAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UTetheredGameplayAbility* TetheredAbility = Cast<UTetheredGameplayAbility>(AbilitySpec.Ability))
		{
			FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
			DynamicTags.AddTag(TetheredAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}




void UTetheredAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	//TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag)); // 
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
		//if (DynamicTags.HasAllExact(InputTags)) // For switching to container
		if (DynamicTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UTetheredAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& Tag)
{
    if (!Tag.IsValid()) return;
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
    {
        if (!Spec.GetDynamicSpecSourceTags().HasTagExact(Tag)) continue;
        AbilitySpecInputPressed(Spec);              // mark pressed FIRST
        TryActivateAbility(Spec.Handle);            // then activate
        if (Spec.IsActive()) AbilitySpecInputPressed(Spec); // belt & suspenders
    }
}

void UTetheredAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& Tag)
{
	if (!Tag.IsValid()) return;

	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.IsActive() && Spec.GetDynamicSpecSourceTags().HasTagExact(Tag))
		{
			AbilitySpecInputReleased(Spec);     // fires GA “Input Released” + WaitInputRelease
		}
	}
}


void UTetheredAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec,
	FActiveGameplayEffectHandle ActiveHandle) const
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}


void UTetheredAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.IsActive())
	{
		// For instanced abilities, use the instance's CurrentActivationInfo
		// (You can iterate all instances; usually there's just one.)
		for (UGameplayAbility* AbilityInst : Spec.GetAbilityInstances())
		{
			if (!AbilityInst) continue;

			const FPredictionKey PredKey =
				AbilityInst->GetCurrentActivationInfo().GetActivationPredictionKey();

			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputPressed,
				Spec.Handle,
				PredKey);
		}
	}
}

void UTetheredAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		for (UGameplayAbility* AbilityInst : Spec.GetAbilityInstances())
		{
			if (!AbilityInst) continue;

			const FPredictionKey PredKey =
				AbilityInst->GetCurrentActivationInfo().GetActivationPredictionKey();

			InvokeReplicatedEvent(
				EAbilityGenericReplicatedEvent::InputReleased,
				Spec.Handle,
				PredKey);
		}
	}
}
