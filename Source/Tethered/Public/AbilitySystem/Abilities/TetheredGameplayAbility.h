// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TetheredGameplayAbility.generated.h"

class UTetheredAbilitySystemComponent;

/**
 * Base Gameplay Ability class for Tethered project
 * Provides common functionality and interface for all abilities in the game
 */
UCLASS()
class TETHERED_API UTetheredGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/** Input tag that can be used to trigger this ability */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag StartupInputTag;


};