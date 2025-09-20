// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class ATetheredCharacter;
class UCombatLifeBar;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TETHERED_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Health Properties
public:
	/** Max amount of HP the character will have on respawn */
	UPROPERTY(EditAnywhere, Category="Health", meta = (ClampMin = 0, ClampMax = 100))
	float MaxHP = 5.0f;

	/** Current amount of HP the character has */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
	float CurrentHP = 0.0f;

	/** Life bar widget fill color */
	UPROPERTY(EditAnywhere, Category="Health")
	FLinearColor LifeBarColor;

	/** Name of the pelvis bone, for damage ragdoll physics */
	UPROPERTY(EditAnywhere, Category="Health")
	FName PelvisBoneName;

	/** Time to wait before respawning the character */
	UPROPERTY(EditAnywhere, Category="Health", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 3.0f;

	/** Camera boom length while the character is dead */
	UPROPERTY(EditAnywhere, Category="Health", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float DeathCameraDistance = 400.0f;
#pragma endregion Health Properties

#pragma region Internal State
private:
	/** Reference to the owning character */
	UPROPERTY()
	ATetheredCharacter* OwnerCharacter;

	/** Pointer to the life bar widget */
	UPROPERTY()
	UCombatLifeBar* LifeBarWidget;

	/** Character respawn timer */
	FTimerHandle RespawnTimer;

	/** Flag to ignore damage (during dash, death, etc.) */
	bool bIgnoreDamage = false;
#pragma endregion Internal State

#pragma region Core Interface
public:
	UHealthComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** Initialize the component with its owner */
	void Initialize(ATetheredCharacter* InOwnerCharacter, UCombatLifeBar* InLifeBarWidget);

	/** Check if currently ignoring damage */
	UFUNCTION(BlueprintPure, Category="Health")
	bool IsIgnoringDamage() const { return bIgnoreDamage; }

	/** Set damage ignore state */
	UFUNCTION(BlueprintCallable, Category="Health")
	void SetIgnoreDamage(bool bShouldIgnore) { bIgnoreDamage = bShouldIgnore; }

	/** Check if character is alive */
	UFUNCTION(BlueprintPure, Category="Health")
	bool IsAlive() const { return CurrentHP > 0.0f; }

	/** Get health percentage */
	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercentage() const { return MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f; }
#pragma endregion Core Interface

#pragma region Health Management
public:
	/** Resets the character's current HP to maximum */
	UFUNCTION(BlueprintCallable, Category="Health")
	void ResetHP();

	/** Modify health by amount (positive for healing, negative for damage) */
	UFUNCTION(BlueprintCallable, Category="Health")
	void ModifyHealth(float Amount, AActor* Instigator = nullptr);

	/** Handles damage and knockback events */
	void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse);

	/** Handles healing events */
	void ApplyHealing(float Healing, AActor* Healer);

	/** Handles death events */
	void HandleDeath();

protected:

	/** Called from the respawn timer to destroy and re-create the character */
	void RespawnCharacter();

	/** Updates the life bar UI */
	void UpdateLifeBarUI();
#pragma endregion Health Management
};