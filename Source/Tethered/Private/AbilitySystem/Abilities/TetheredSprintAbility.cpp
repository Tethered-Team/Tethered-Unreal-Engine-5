// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/Abilities/TetheredSprintAbility.h"
#include "AbilitySystem/TetheredAbilitySystemComponent.h"
#include "AbilitySystem/TetheredAttributeSet.h"
#include "GameplayEffect.h"
#include "Engine/World.h"
#include "TimerManager.h"

UTetheredSprintAbility::UTetheredSprintAbility()
{
	// Set default values
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// This ability can be cancelled by other abilities or when stamina runs out
	bRetriggerInstancedAbility = false;
	
	// Set default sprint tag
	SprintTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.Sprinting"));
}

void UTetheredSprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Check if we have enough stamina to start sprinting
	if (!CanContinueSprinting())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Apply the sprint tag
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(SprintTag);
		
		// Apply sprint speed effect if specified
		if (SprintSpeedEffect)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);
			
			FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(SprintSpeedEffect, 1.0f, EffectContext);
			if (EffectSpecHandle.IsValid())
			{
				SprintEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
			}
		}
	}

	// Start stamina consumption timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			StaminaConsumptionTimer,
			this,
			&UTetheredSprintAbility::ConsumeStamina,
			1.0f, // Every second
			true  // Looping
		);
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UTetheredSprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear stamina consumption timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StaminaConsumptionTimer);
	}

	// Remove sprint tag and effect
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(SprintTag);
		
		// Remove sprint speed effect
		if (SprintEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(SprintEffectHandle);
			SprintEffectHandle = FActiveGameplayEffectHandle();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTetheredSprintAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// End sprint when input is released
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UTetheredSprintAbility::ConsumeStamina()
{
	// Check if we can continue sprinting
	if (!CanContinueSprinting())
	{
		// End ability if we can't continue
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Apply stamina cost
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		// Create a simple instant effect to reduce stamina
		// In a real implementation, you'd want to create a proper GameplayEffect for this
		if (const UTetheredAttributeSet* AttributeSet = Cast<UTetheredAttributeSet>(ASC->GetAttributeSet(UTetheredAttributeSet::StaticClass())))
		{
			// This is a simplified approach - you should use a GameplayEffect for stamina consumption
			// ASC->ApplyModToAttribute(AttributeSet->GetStaminaAttribute(), EGameplayModOp::Additive, -StaminaCostPerSecond);
		}
	}
}

bool UTetheredSprintAbility::CanContinueSprinting() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (const UTetheredAttributeSet* AttributeSet = Cast<UTetheredAttributeSet>(ASC->GetAttributeSet(UTetheredAttributeSet::StaticClass())))
		{
			// Check if we have enough stamina (assuming you have a stamina attribute)
			// return AttributeSet->GetStamina() >= MinimumStaminaToStart;
			
			// For now, just return true since stamina isn't implemented yet
			return true;
		}
	}
	
	return false;
}