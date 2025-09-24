// Copyright Epic Games, Inc. All Rights Reserved.

#include "AbilitySystem/TetheredAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "TetheredGameplayTags.h"
#include "GameFramework/Character.h"


UTetheredAttributeSet::UTetheredAttributeSet()
{
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Primary_Health.GetTag(), UTetheredAttributeSet::GetHealthAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Primary_MaxHealth.GetTag(), UTetheredAttributeSet::GetMaxHealthAttribute);

	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Secondary_HealthRegen.GetTag(), UTetheredAttributeSet::GetHealthRegenRateAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Secondary_MovementSpeed.GetTag(), UTetheredAttributeSet::GetMovementSpeedAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Secondary_SprintMultiplier.GetTag(), UTetheredAttributeSet::GetSprintMultiplierAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Combat_AttackDamage.GetTag(), UTetheredAttributeSet::GetAttackDamageAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Combat_AttackSpeed.GetTag(), UTetheredAttributeSet::GetAttackSpeedAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Combat_CriticalChance.GetTag(), UTetheredAttributeSet::GetCritChanceAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Combat_CriticalDamage.GetTag(), UTetheredAttributeSet::GetCritMultiplierAttribute);
	TagsToAttributes.Add(TetheredGameplayTags::TAG_Attribute_Resistance_Physical.GetTag(), UTetheredAttributeSet::GetDamageResistanceAttribute);

}

void UTetheredAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, HealthRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, SprintMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, CritChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, CritMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTetheredAttributeSet, DamageResistance, COND_None, REPNOTIFY_Always);
}

void UTetheredAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp health to 0 minimum and max health maximum
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	// Ensure max health is never below 1
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	// Clamp movement speed to reasonable values
	else if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 2000.0f);
	}
	// Clamp sprint multiplier
	else if (Attribute == GetSprintMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.0f, 3.0f);
	}
	// Clamp attack damage to non-negative
	else if (Attribute == GetAttackDamageAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	// Clamp attack speed to reasonable values
	else if (Attribute == GetAttackSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.1f, 5.0f);
	}
	// Clamp crit chance to 0-100%
	else if (Attribute == GetCritChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	}
	// Clamp crit multiplier to reasonable values
	else if (Attribute == GetCritMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.0f, 10.0f);
	}
	// Clamp damage resistance to 0-95%
	else if (Attribute == GetDamageResistanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 0.95f);
	}
}

void UTetheredAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle health changes
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		// Check for death
		if (GetHealth() <= 0.0f)
		{
			if (AActor* TargetActor = GetOwningActor())
			{
				// Broadcast death event or handle death logic
				UE_LOG(LogTemp, Warning, TEXT("Character %s has died!"), *TargetActor->GetName());
			}
		}
	}
}


float UTetheredAttributeSet::GetHealthPercent() const
{
	return GetMaxHealth() > 0.0f ? GetHealth() / GetMaxHealth() : 0.0f;
}

bool UTetheredAttributeSet::IsAtFullHealth() const
{
	return FMath::IsNearlyEqual(GetHealth(), GetMaxHealth(), 0.01f);
}

bool UTetheredAttributeSet::IsAtLowHealth() const
{
	return GetHealthPercent() <= 0.25f;
}

bool UTetheredAttributeSet::IsAtCriticalHealth() const
{
	return GetHealthPercent() <= 0.1f;
}


#pragma region RepNotify Functions
void UTetheredAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, Health, OldHealth);
}

void UTetheredAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, MaxHealth, OldMaxHealth);
}

void UTetheredAttributeSet::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, HealthRegenRate, OldHealthRegenRate);
}

void UTetheredAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, MovementSpeed, OldMovementSpeed);
}

void UTetheredAttributeSet::OnRep_SprintMultiplier(const FGameplayAttributeData& OldSprintMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, SprintMultiplier, OldSprintMultiplier);
}

void UTetheredAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& OldAttackDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, AttackDamage, OldAttackDamage);
}

void UTetheredAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, AttackSpeed, OldAttackSpeed);
}

void UTetheredAttributeSet::OnRep_CritChance(const FGameplayAttributeData& OldCritChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, CritChance, OldCritChance);
}

void UTetheredAttributeSet::OnRep_CritMultiplier(const FGameplayAttributeData& OldCritMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, CritMultiplier, OldCritMultiplier);
}

void UTetheredAttributeSet::OnRep_DamageResistance(const FGameplayAttributeData& OldDamageResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTetheredAttributeSet, DamageResistance, OldDamageResistance);
}

#pragma endregion RepNotify Functions


void UTetheredAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& EffectProperties)
{
	const FGameplayEffectContextHandle EffectContextHandle = Data.EffectSpec.GetContext();
	EffectProperties.SourceASC = EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

	if (IsValid(EffectProperties.SourceASC) && EffectProperties.SourceASC->AbilityActorInfo.IsValid() && EffectProperties.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		EffectProperties.SourceAvatarActor = EffectProperties.SourceASC->AbilityActorInfo->AvatarActor.Get();
		EffectProperties.SourceController = EffectProperties.SourceASC->AbilityActorInfo->PlayerController.Get();

		if (EffectProperties.SourceController == nullptr && EffectProperties.SourceAvatarActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(EffectProperties.SourceAvatarActor))
			{
				EffectProperties.SourceController = Pawn->GetController();
			}
		}
		if (EffectProperties.SourceController)
		{
			EffectProperties.SourceCharacter = Cast<ACharacter>(EffectProperties.SourceController->GetPawn());
		}

	}

	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		EffectProperties.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		EffectProperties.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		EffectProperties.TargetCharacter = Cast<ACharacter>(EffectProperties.TargetCharacter);
		EffectProperties.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(EffectProperties.TargetAvatarActor);
	}


}