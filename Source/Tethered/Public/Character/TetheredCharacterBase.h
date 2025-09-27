// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "TetheredCharacterBase.generated.h"

// Forward declarations
class UGameplayEffect;
class UGameplayAbility;
struct FOnAttributeChangeData;

DECLARE_LOG_CATEGORY_EXTERN(LogTetheredCharacter, Log, All);

/**
 * Refactored Combat Character using component-based architecture with AbilitySystem integration
 * This class focuses on coordination between components and ability execution
 * Input is routed through the PlayerController which activates abilities on this character
 */
UCLASS(Abstract)
class TETHERED_API ATetheredCharacterBase : public ACharacter, public IAbilitySystemInterface//, public ICombatInterface
{
	GENERATED_BODY()

public:
	ATetheredCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	virtual void BeginPlay() override;

	void BindMovementAttributes();

	void RecalcMaxSpeed(const struct FOnAttributeChangeData&);

	virtual void InitAbilityActorInfo();

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	//virtual FVector GetCombatSocketLocation() const override;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	void ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& GameplayEffectClass, float Level) const;

	virtual void InitializeDefaultAttributes() const;

	void AddCharacterAbilities();


private:
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;



};

