// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedMovementComponent.generated.h"

class ATetheredCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TETHERED_API UEnhancedMovementComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Movement Properties
public:
	/** Base walking speed */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm/s"))
	float MaxWalkSpeed = 500.0f;

	/** Running speed */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm/s"))
	float MaxRunSpeed = 900.0f;

	/** Dash transition time */
	UPROPERTY(EditAnywhere, Category="Movement", meta = (ClampMin = 0.1, ClampMax = 2.0, Units = "s"))
	float DashToRunTransitionTime = 0.5f;

	/** Speed retention after dash ends */
	UPROPERTY(EditAnywhere, Category="Movement", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DashMomentumRetention = 0.6f;
#pragma endregion Movement Properties

#pragma region Internal State
private:
	/** Reference to the owning character */
	UPROPERTY()
	ATetheredCharacter* OwnerCharacter;

	/** Cached reference to the character's movement component */
	UPROPERTY()
	UCharacterMovementComponent* CharacterMovement;

	/** Current movement speed (used for smooth transitions) */
	float CurrentMovementSpeed = 0.0f;

	/** Target movement speed for smooth transitions */
	float TargetMovementSpeed = 0.0f;

	/** Flag to indicate if we're in a dash transition */
	bool bIsInDashTransition = false;

	/** Flag for run input state */
	bool bRunHeld = false;
#pragma endregion Internal State

#pragma region Core Interface
public:
	UEnhancedMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initialize the component with its owner */
	void Initialize(ATetheredCharacter* InOwnerCharacter);

	/** Check if currently in dash transition */
	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsInDashTransition() const { return bIsInDashTransition; }

	/** Check if run is held */
	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsRunHeld() const { return bRunHeld; }

	/** Get the wrapped character movement component */
	UFUNCTION(BlueprintPure, Category="Movement")
	UCharacterMovementComponent* GetCharacterMovement() const { return CharacterMovement; }
#pragma endregion Core Interface

#pragma region Movement Actions
public:
	/** Handle movement input */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void DoMove(float Right, float Forward);

	/** Set run state */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void SetRunState(bool bShouldRun);

	/** Start dash to movement transition */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StartDashToMovementTransition(float DashSpeed);

	/** Stop dash transition */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void StopDashTransition();

	/** Updates movement speed based on current input state */
	void UpdateMovementSpeed();

protected:
	/** Updates smooth movement speed transitions */
	void UpdateMovementSpeedTransition(float DeltaTime);
	


	/** Get the forward direction based on camera */
	FVector GetForwardDirection() const;

	/** Get the right direction based on camera */
	FVector GetRightDirection() const;
#pragma endregion Movement Actions

#pragma region Enhanced Movement Features
public:
	/** Apply movement input with enhanced features (camera-relative, rotation, etc.) */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void ApplyEnhancedMovementInput(float Forward, float Right);

	/** Set movement speed directly (useful for abilities, effects, etc.) */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void SetMovementSpeed(float NewSpeed);

	/** Get current effective movement speed */
	UFUNCTION(BlueprintPure, Category="Movement")
	float GetCurrentMovementSpeed() const { return CurrentMovementSpeed; }

	/** Check if character is moving */
	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsMoving() const;

	/** Get movement direction as normalized vector */
	UFUNCTION(BlueprintPure, Category="Movement")
	FVector GetMovementDirection() const;
#pragma endregion Enhanced Movement Features
};