// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerMovementComponent.generated.h"

class ATetheredCharacter;
class UCameraComponent;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
	Idle,
	Walking,
	Running,
	Dashing,
	InAir
};

/**
 * Self-contained Player Movement Component that handles all player movement including dash
 * No longer depends on separate Enhanced and Dash components
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TETHERED_API UPlayerMovementComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Movement Settings
public:
	/** Base walk speed */
	UPROPERTY(EditAnywhere, Category="Movement Speed", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm/s"))
	float BaseWalkSpeed = 500.0f;

	/** Run speed multiplier */
	UPROPERTY(EditAnywhere, Category="Movement Speed", meta = (ClampMin = 1.0, ClampMax = 5.0))
	float RunSpeedMultiplier = 1.5f;

	/** Movement acceleration */
	UPROPERTY(EditAnywhere, Category="Movement Speed", meta = (ClampMin = 100, ClampMax = 5000))
	float MovementAcceleration = 2000.0f;

	/** Movement deceleration */
	UPROPERTY(EditAnywhere, Category="Movement Speed", meta = (ClampMin = 100, ClampMax = 5000))
	float MovementDeceleration = 2000.0f;

	/** Should movement input be relative to camera direction */
	UPROPERTY(EditAnywhere, Category="Movement")
	bool bUseCameraRelativeMovement = true;

	/** Should character rotate to face movement direction */
	UPROPERTY(EditAnywhere, Category="Movement")
	bool bRotateToMovementDirection = true;

	/** Rotation interpolation speed */
	UPROPERTY(EditAnywhere, Category="Movement", meta = (ClampMin = 0.1, ClampMax = 50.0))
	float RotationInterpSpeed = 15.0f;
#pragma endregion Movement Settings

#pragma region Dash Settings
public:
	/** Dash speed */
	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 500, ClampMax = 5000, Units = "cm/s"))
	float DashSpeed = 2000.0f;

	/** Dash duration */
	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0.1, ClampMax = 2.0, Units = "s"))
	float DashDuration = 0.3f;

	/** Dash cooldown */
	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0.0, ClampMax = 10.0, Units = "s"))
	float DashCooldown = 1.0f;

	/** Should dash in movement direction or forward */
	UPROPERTY(EditAnywhere, Category="Dash")
	bool bDashInMovementDirection = true;

	/** Input buffer time for dash */
	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0.0, ClampMax = 1.0, Units = "s"))
	float DashInputBufferTime = 0.2f;
#pragma endregion Dash Settings

#pragma region State
private:
	/** Reference to owning character */
	UPROPERTY()
	ATetheredCharacter* OwnerCharacter;

	/** Cached character movement component */
	UPROPERTY()
	UCharacterMovementComponent* CharacterMovement;

	/** Cached camera reference */
	UPROPERTY()
	UCameraComponent* FollowCamera;

	/** Current movement state */
	EPlayerMovementState CurrentMovementState = EPlayerMovementState::Idle;

	/** Current movement input */
	FVector2D CurrentMovementInput = FVector2D::ZeroVector;

	/** Last movement direction */
	FVector LastMovementDirection = FVector::ZeroVector;

	/** Is running */
	bool bIsRunning = false;

	/** Dash state */
	bool bIsDashing = false;
	float DashTimeRemaining = 0.0f;
	float LastDashTime = -1000.0f;
	FVector DashDirection = FVector::ZeroVector;

	/** Input buffering */
	bool bDashInputBuffered = false;
	float DashInputBufferStartTime = 0.0f;

	/** Speed overrides */
	bool bHasSpeedOverride = false;
	float SpeedOverrideMultiplier = 1.0f;
	float SpeedOverrideEndTime = 0.0f;
#pragma endregion State

#pragma region Core Interface
public:
	UPlayerMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initialize with owner character */
	void Initialize(ATetheredCharacter* InOwnerCharacter);

	/** Get current movement state */
	UFUNCTION(BlueprintPure, Category="Movement")
	EPlayerMovementState GetMovementState() const { return CurrentMovementState; }

	/** Check if moving */
	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsMoving() const;

	/** Check if dashing */
	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsDashing() const { return bIsDashing; }

	/** Check if running */
	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsRunning() const { return bIsRunning; }

	/** Get movement direction */
	UFUNCTION(BlueprintPure, Category="Movement")
	FVector GetMovementDirection() const { return LastMovementDirection; }
#pragma endregion Core Interface

#pragma region Input Handling
public:
	/** Handle movement input */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void HandleMovementInput(const FInputActionValue& Value);

	/** Handle dash input */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void HandleDashInput();

	/** Set run state */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void SetRunState(bool bShouldRun);

	/** Direct movement input */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void SetMovementInput(float Forward, float Right);

	/** Stop all movement */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StopAllMovement();

protected:
	/** Process movement input */
	void ProcessMovementInput(float Forward, float Right);

	/** Update character rotation */
	void UpdateCharacterRotation(const FVector& MovementDirection, float DeltaTime);

	/** Calculate camera-relative directions */
	void GetCameraRelativeDirections(FVector& OutForward, FVector& OutRight) const;
#pragma endregion Input Handling

#pragma region Dash System
protected:
	/** Start dash */
	void StartDash();

	/** Update dash */
	void UpdateDash(float DeltaTime);

	/** End dash */
	void EndDash();

	/** Check if can dash */
	bool CanDash() const;

	/** Calculate dash direction */
	FVector CalculateDashDirection() const;

	/** Process dash input buffer */
	void ProcessDashInputBuffer(float DeltaTime);
#pragma endregion Dash System

#pragma region Speed Management
public:
	/** Set movement speed override */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void SetMovementSpeedOverride(float SpeedMultiplier, float Duration = -1.0f);

	/** Clear movement speed override */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void ClearMovementSpeedOverride();

	/** Get effective movement speed */
	UFUNCTION(BlueprintPure, Category="Movement")
	float GetEffectiveMovementSpeed() const;

	/** Check if movement is currently overridden */
	UFUNCTION(BlueprintPure, Category="Movement")
	bool HasMovementOverride() const { return bHasSpeedOverride; }

protected:
	/** Update movement speed based on current state */
	void UpdateMovementSpeed();

	/** Update speed overrides */
	void UpdateSpeedOverrides(float DeltaTime);
#pragma endregion Speed Management

#pragma region State Management
protected:
	/** Update movement state */
	void UpdateMovementState();

	/** Handle state transitions */
	void OnMovementStateChanged(EPlayerMovementState OldState, EPlayerMovementState NewState);
#pragma endregion State Management

#pragma region Blueprint Events
public:
	/** Called when movement state changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Movement")
	void OnMovementStateChangedBP(EPlayerMovementState OldState, EPlayerMovementState NewState);

	/** Called when dash starts */
	UFUNCTION(BlueprintImplementableEvent, Category="Movement")
	void OnDashStarted(const FVector& NewDashDirection);

	/** Called when dash ends */
	UFUNCTION(BlueprintImplementableEvent, Category="Movement")
	void OnDashEnded();

	/** Called when movement input is received */
	UFUNCTION(BlueprintImplementableEvent, Category="Movement")
	void OnMovementInputReceived(const FVector2D& InputVector);
#pragma endregion Blueprint Events
};