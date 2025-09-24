// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TetheredAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

USTRUCT()
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() {};

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;

};
//typedef is specific to the FGameplayAttribute() signature, but TStaticFuncPtr is a template that can be used for any signature.
//typedef TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr FAttributeFuncPtr;
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;



/**
 * Base attribute set for most Tethered characters (players, enemies, NPCs)
 * Contains essential health, movement, and combat stats that all characters need
 */
UCLASS(BlueprintType)
class TETHERED_API UTetheredAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTetheredAttributeSet();

	// AttributeSet overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;

#pragma region Health Attributes
	/** Current health, when 0 we expect owner to die unless prevented by an ability. Capped by MaxHealth. */
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, Health)

	/** MaxHealth is the maximum health this character can have. Health cannot exceed MaxHealth. */
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, MaxHealth)

	/** Health regeneration rate per second */
	UPROPERTY(BlueprintReadOnly, Category = "Health", ReplicatedUsing = OnRep_HealthRegenRate)
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, HealthRegenRate)
#pragma endregion Health Attributes

#pragma region Movement Attributes
	/** Maximum movement speed */
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, MovementSpeed)

	/** Movement speed multiplier for running/sprinting */
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_SprintMultiplier)
	FGameplayAttributeData SprintMultiplier;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, SprintMultiplier)
#pragma endregion Movement Attributes

#pragma region Combat Attributes
	/** Base attack damage */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, AttackDamage)

	/** Attack speed multiplier (higher = faster attacks) */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, AttackSpeed)

	/** Critical hit chance (0.0 - 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CritChance)
	FGameplayAttributeData CritChance;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, CritChance)

	/** Critical hit damage multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_CritMultiplier)
	FGameplayAttributeData CritMultiplier;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, CritMultiplier)

	/** Damage resistance (0.0 - 1.0, reduces incoming damage) */
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_DamageResistance)
	FGameplayAttributeData DamageResistance;
	ATTRIBUTE_ACCESSORS(UTetheredAttributeSet, DamageResistance)
#pragma endregion Combat Attributes

#pragma region Helper Functions
public:
	/** Get health percentage (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Tethered|Health")
	float GetHealthPercent() const;

	/** Check if character is at full health */
	UFUNCTION(BlueprintPure, Category = "Tethered|Health")
	bool IsAtFullHealth() const;

	/** Check if character is at low health (below 25%) */
	UFUNCTION(BlueprintPure, Category = "Tethered|Health")
	bool IsAtLowHealth() const;

	/** Check if character is critically low on health (below 10%) */
	UFUNCTION(BlueprintPure, Category = "Tethered|Health")
	bool IsAtCriticalHealth() const;
#pragma endregion Helper Functions

#pragma region RepNotify Functions
protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate);

	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

	UFUNCTION()
	virtual void OnRep_SprintMultiplier(const FGameplayAttributeData& OldSprintMultiplier);

	UFUNCTION()
	virtual void OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage);

	UFUNCTION()
	virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);

	UFUNCTION()
	virtual void OnRep_CritChance(const FGameplayAttributeData& OldCritChance);

	UFUNCTION()
	virtual void OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier);

	UFUNCTION()
	virtual void OnRep_DamageResistance(const FGameplayAttributeData& OldDamageResistance);
#pragma endregion RepNotify Functions

private:
	static void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties);
};