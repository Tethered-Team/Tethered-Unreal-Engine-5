// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/CombatAttacker.h"
#include "Animation/AnimInstance.h"
#include "CombatComponent.generated.h"


DECLARE_DELEGATE_TwoParams(FOnAttackMontageEnded, UAnimMontage*, bool);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TETHERED_API UCombatComponent : public UActorComponent//, public ICombatAttacker
{
	GENERATED_BODY()

//#pragma region Combat Properties
//public:
//	/** Max amount of time that may elapse for a non-combo attack input to not be considered stale */
//	UPROPERTY(EditAnywhere, Category="Melee Attack", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
//	float AttackInputCacheTimeTolerance = 1.0f;
//
//	/** Distance ahead of the character that melee attack sphere collision traces will extend */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units="cm"))
//	float MeleeTraceDistance = 75.0f;
//
//	/** Radius of the sphere trace for melee attacks */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
//	float MeleeTraceRadius = 75.0f;
//
//	/** Amount of damage a melee attack will deal */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 100))
//	float MeleeDamage = 1.0f;
//
//	/** Amount of knockback impulse a melee attack will apply */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
//	float MeleeKnockbackImpulse = 250.0f;
//
//	/** Amount of upwards impulse a melee attack will apply */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
//	float MeleeLaunchImpulse = 300.0f;
//
//	/** AnimMontage that will play for combo attacks */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
//	UAnimMontage* ComboAttackMontage;
//
//	/** Names of the AnimMontage sections that correspond to each stage of the combo attack */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
//	TArray<FName> ComboSectionNames;
//
//	/** Max amount of time that may elapse for a combo attack input to not be considered stale */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
//	float ComboInputCacheTimeTolerance = 0.45f;
//
//	/** AnimMontage that will play for charged attacks */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
//	UAnimMontage* ChargedAttackMontage;
//
//	/** Name of the AnimMontage section that corresponds to the charge loop */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
//	FName ChargeLoopSection;
//
//	/** Name of the AnimMontage section that corresponds to the attack */
//	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
//	FName ChargeAttackSection;
//
//	/** Whether to show debug traces for combat attacks */
//	UPROPERTY(EditAnywhere, Category="Debug", meta = (DisplayName = "Show Attack Traces"))
//	bool bDebugShowTraces = false;
//#pragma endregion Combat Properties
//
//#pragma region Internal State
//private:
//	/** Time at which an attack button was last pressed */
//	float CachedAttackInputTime = 0.0f;
//
//	/** If true, the character is currently playing an attack animation */
//	bool bIsAttacking = false;
//
//	/** Index of the current stage of the melee attack combo */
//	int32 ComboCount = 0;
//
//	/** Flag that determines if the player is currently holding the charged attack input */
//	bool bIsChargingAttack = false;
//	
//	/** If true, the charged attack hold check has been tested at least once */
//	bool bHasLoopedChargedAttack = false;
//
//	/** Reference to the owning character */
//	UPROPERTY()
//	ATetheredPlayerCharacter* OwnerCharacter;
//
//	/** Attack montage ended delegate */
//	FOnMontageEnded OnAttackMontageEnded;
//#pragma endregion Internal State
//
//#pragma region Core Interface
//public:
//	UCombatComponent();
//
//protected:
//	virtual void BeginPlay() override;
//
//public:
//	/** Initialize the component with its owner */
//	void Initialize(ATetheredPlayerCharacter* InOwnerCharacter);
//
//	/** Gets the current attacking state */
//	UFUNCTION(BlueprintPure, Category="Combat")
//	bool IsAttacking() const { return bIsAttacking; }
//
//	/** Gets the current combo count */
//	UFUNCTION(BlueprintPure, Category="Combat")
//	int32 GetComboCount() const { return ComboCount; }
//
//	/** Gets whether currently charging an attack */
//	UFUNCTION(BlueprintPure, Category="Combat")
//	bool IsChargingAttack() const { return bIsChargingAttack; }
//#pragma endregion Core Interface
//
//#pragma region Combat Actions
//public:
//	/** Handles combo attack pressed */
//	UFUNCTION(BlueprintCallable, Category="Combat")
//	void DoComboAttackStart();
//
//	/** Handles combo attack released */
//	UFUNCTION(BlueprintCallable, Category="Combat")
//	void DoComboAttackEnd();
//
//	/** Handles charged attack pressed */
//	UFUNCTION(BlueprintCallable, Category="Combat")
//	void DoChargedAttackStart();
//
//	/** Handles charged attack released */
//	UFUNCTION(BlueprintCallable, Category="Combat")
//	void DoChargedAttackEnd();
//
//protected:
//	/** Performs a combo attack */
//	void ComboAttack();
//
//	/** Performs a charged attack */
//	void ChargedAttack();
//
//	/** Called from a delegate when the attack montage ends */
//	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
//#pragma endregion Combat Actions
//
//#pragma region ICombatAttacker Interface
//public:
//	/** Performs the collision check for an attack */
//	virtual void DoAttackTrace(FName DamageSourceBone) override;
//
//	/** Performs the combo string check */
//	virtual void CheckCombo() override;
//
//	/** Performs the charged attack hold check */
//	virtual void CheckChargedAttack() override;
//#pragma endregion ICombatAttacker Interface
//
//#pragma region Debug
//private:
//	/** Checks if global combat debug is enabled via CheatManager */
//	static bool IsGlobalCombatDebugEnabled();
//#pragma endregion Debug
};

