// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatAttacker.h"
#include "Interfaces/CombatDamageable.h"
#include "TetheredCharacter.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UCombatLifeBar;
class UWidgetComponent;
class UCombatComponent;
class UHealthComponent;
class UPlayerMovementComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTetheredCharacter, Log, All);

/**
 * Refactored Combat Character using component-based architecture
 * This class focuses on coordination between components rather than implementing all features directly
 */
UCLASS(abstract)
class TETHERED_API ATetheredCharacter : public ACharacter, public ICombatAttacker, public ICombatDamageable
{
	GENERATED_BODY()

#pragma region Components
private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Life bar widget component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* LifeBar;

	/** Combat system component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	/** Health system component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthComponent* HealthComponent;

	/** Unified player movement component that handles all movement including dash */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UPlayerMovementComponent* PlayerMovementComponent;
#pragma endregion Components

#pragma region Input Actions
protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

	/** Combo Attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ComboAttackAction;

	/** Charged Attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ChargedAttackAction;

	/** Dash Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DashAction;

	/** Run Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* RunAction;
#pragma endregion Input Actions

#pragma region Camera and Respawn
protected:
	/** Camera boom length when the character respawns */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float DefaultCameraDistance = 100.0f;

	/** Copy of the mesh's transform so we can reset it after ragdoll animations */
	FTransform MeshStartingTransform;
#pragma endregion Camera and Respawn

#pragma region Core Interface
public:
	/** Constructor */
	ATetheredCharacter();

	/** Called every frame */
	virtual void Tick(float DeltaTime) override;

protected:
	/** Initialization */
	virtual void BeginPlay() override;

	/** Cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Handles input bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Handles possessed initialization */
	virtual void NotifyControllerChanged() override;

	/** Overrides landing to reset damage ragdoll physics */
	virtual void Landed(const FHitResult& Hit) override;

	/** Overrides the default TakeDamage functionality */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
#pragma endregion Core Interface

#pragma region Input Handlers
protected:
	/** Called for movement input - now routes through PlayerMovementComponent */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for combo attack input */
	void ComboAttackPressed();

	/** Called for charged attack input pressed */
	void ChargedAttackPressed();

	/** Called for charged attack input released */
	void ChargedAttackReleased();

	/** Called for dash input - now routes through PlayerMovementComponent */
	void Dash();

	/** Called for run input pressed */
	void RunPressed();

	/** Called for run input released */
	void RunReleased();

public:
	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);
#pragma endregion Input Handlers

#pragma region Component Accessors
public:
	/** Get the combat component */
	UFUNCTION(BlueprintPure, Category = "Components")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	/** Get the health component */
	UFUNCTION(BlueprintPure, Category = "Components")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	/** Get the player movement component */
	UFUNCTION(BlueprintPure, Category = "Components")
	UPlayerMovementComponent* GetPlayerMovementComponent() const { return PlayerMovementComponent; }

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Get the mesh starting transform for respawn */
	FORCEINLINE const FTransform& GetMeshStartingTransform() const { return MeshStartingTransform; }

	/** Get the default camera distance */
	FORCEINLINE float GetDefaultCameraDistance() const { return DefaultCameraDistance; }
#pragma endregion Component Accessors

#pragma region ICombatAttacker Interface
public:
	/** Performs the collision check for an attack - delegates to CombatComponent */
	virtual void DoAttackTrace(FName DamageSourceBone) override;

	/** Performs the combo string check - delegates to CombatComponent */
	virtual void CheckCombo() override;

	/** Performs the charged attack hold check - delegates to CombatComponent */
	virtual void CheckChargedAttack() override;
#pragma endregion ICombatAttacker Interface

#pragma region ICombatDamageable Interface
public:
	/** Handles damage and knockback events - delegates to HealthComponent */
	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;

	/** Handles healing events - delegates to HealthComponent */
	virtual void ApplyHealing(float Healing, AActor* Healer) override;

	/** Handles death events - delegates to HealthComponent */
	virtual void HandleDeath() override;
#pragma endregion ICombatDamageable Interface

#pragma region Blueprint Events
public:
	/** Blueprint handler to play damage received effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta = (DisplayName = "On Received Damage"))
	void OnReceivedDamage(float Damage, const FVector& ImpactPoint, const FVector& DamageDirection);

	/** Blueprint handler to play damage dealt effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta = (DisplayName = "On Dealt Damage"))
	void OnDealtDamage(float Damage, const FVector& ImpactPoint);

	/** Blueprint handler for death events */
	UFUNCTION(BlueprintImplementableEvent, Category="Health", meta = (DisplayName = "On Death"))
	void OnDeath();

	/** Blueprint handler for respawn events */
	UFUNCTION(BlueprintImplementableEvent, Category="Health", meta = (DisplayName = "On Respawn"))
	void OnRespawn();
#pragma endregion Blueprint Events
};