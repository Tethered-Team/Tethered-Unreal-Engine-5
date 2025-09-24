// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/TetheredGameplayAbility.h"
#include "TetheredSprintAbility.generated.h"

/**
 * Sprint ability that:
 * 1. Applies a gameplay effect to modify sprint multiplier
 * 2. Grants the "State.Movement.Sprinting" tag
 * 3. Handles stamina consumption over time
 */
UCLASS()
class TETHERED_API UTetheredSprintAbility : public UTetheredGameplayAbility
{
	GENERATED_BODY()

public:
	UTetheredSprintAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	/** Gameplay effect that modifies sprint speed */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	TSubclassOf<UGameplayEffect> SprintSpeedEffect;

	/** Tag applied while sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	FGameplayTag SprintTag;

	/** Stamina cost per second while sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float StaminaCostPerSecond = 20.0f;

	/** Minimum stamina required to start sprinting */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float MinimumStaminaToStart = 10.0f;

private:
	/** Handle for the applied sprint effect */
	FActiveGameplayEffectHandle SprintEffectHandle;

	/** Timer for stamina consumption */
	FTimerHandle StaminaConsumptionTimer;

	/** Consume stamina while sprinting */
	UFUNCTION()
	void ConsumeStamina();

	/** Check if we can continue sprinting */
	bool CanContinueSprinting() const;
};