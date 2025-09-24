// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "TetheredCharacterData.generated.h"

class UTetheredGameplayAbility;

/**
 * Data asset that defines the starting stats and abilities for a character
 */
UCLASS(BlueprintType)
class TETHERED_API UTetheredCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Starting attribute values */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> PrimaryAttributeDefaults;

	/** Secondary attribute defaults (calculated from primary) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> SecondaryAttributeDefaults;

	/** Vital attribute defaults (health, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<UGameplayEffect> VitalAttributeDefaults;

	/** Starting abilities for this character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UTetheredGameplayAbility>> StartingAbilities;

	/** Effects to apply on character spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<TSubclassOf<UGameplayEffect>> StartingGameplayEffects;
};