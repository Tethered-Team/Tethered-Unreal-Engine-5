// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "DashComponent.generated.h"

class ATetheredPlayerCharacter;
class UEnhancedMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TETHERED_API UDashComponent : public UActorComponent
{
	GENERATED_BODY()

//#pragma region Dash Properties
//public:
//	/** AnimMontage to use for the Dash action */
//	UPROPERTY(EditAnywhere, Category="Dash")
//	UAnimMontage* DashMontage;
//
//	/** Maximum distance the character can dash */
//	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm"))
//	float DashDistance = 400.0f;
//
//	/** Speed at which the character dashes */
//	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0, ClampMax = 5000, Units = "cm/s"))
//	float DashSpeed = 1200.0f;
//
//	/** Radius of the sphere used for dash collision checking */
//	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0, ClampMax = 100, Units = "cm"))
//	float DashCollisionRadius = 30.0f;
//
//	/** Number of collision check points along the dash path */
//	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 5, ClampMax = 50))
//	int32 DashCollisionSteps = 20;
//
//	/** Minimum distance required for a valid dash landing spot */
//	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 10, ClampMax = 200, Units = "cm"))
//	float MinDashDistance = 50.0f;
//
//	/** Time to transition from dash speed to regular movement speed */
//	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0.1, ClampMax = 2.0, Units = "s"))
//	float DashToRunTransitionTime = 0.5f;
//
//	/** Speed retention after dash ends */
//	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0.0, ClampMax = 1.0))
//	float DashMomentumRetention = 0.6f;
//#pragma endregion Dash Properties
//
//#pragma region Internal State
//private:
//	/** movement state flag bits, packed into a uint8 for memory efficiency */
//	uint8 bHasDashed : 1;
//	uint8 bIsDashing : 1;
//
//	/** Current dash target location */
//	FVector DashTargetLocation = FVector::ZeroVector;
//
//	/** Reference to the owning character */
//	UPROPERTY()
//	ATetheredPlayerCharacter* OwnerCharacter;
//
//	/** Reference to the movement component for coordination */
//	UPROPERTY()
//	UEnhancedMovementComponent* MovementComponent;
//
//	/** Dash montage ended delegate */
//	FOnMontageEnded OnDashMontageEnded;
//#pragma endregion Internal State
//
//#pragma region Core Interface
//public:
//	UDashComponent();
//
//protected:
//	virtual void BeginPlay() override;
//	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//
//public:
//	/** Initialize the component with its owner and movement component */
//	void Initialize(ATetheredPlayerCharacter* InOwnerCharacter, UEnhancedMovementComponent* InMovementComponent);
//
//	/** Check if currently dashing */
//	UFUNCTION(BlueprintPure, Category="Dash")
//	bool IsDashing() const { return bIsDashing; }
//
//	/** Check if has dashed (for cooldowns) */
//	UFUNCTION(BlueprintPure, Category="Dash")
//	bool HasDashed() const { return bHasDashed; }
//
//	/** Reset dash state (usually called on landing) */
//	UFUNCTION(BlueprintCallable, Category="Dash")
//	void ResetDashState();
//
//	UFUNCTION(BlueprintPure, Category="Dash")
//	FVector GetDashTargetLocation() const { return DashTargetLocation; }
//
//	float GetDashSpeed() const { return DashSpeed; }
//#pragma endregion Core Interface
//
//#pragma region Dash Actions
//public:
//	/** Handles dash inputs */
//	UFUNCTION(BlueprintCallable, Category="Dash")
//	void DoDash();
//
//	/** Ends the dash state */
//	UFUNCTION(BlueprintCallable, Category="Dash")
//	void EndDash();
//
//protected:
//	/** Called from a delegate when the dash montage ends */
//	void DashMontageEnded(UAnimMontage* Montage, bool bInterrupted);
//#pragma endregion Dash Actions
//
//#pragma region Dash Calculations
//public:
//	/** Calculates the furthest valid dash destination by testing the entire dash length */
//	UFUNCTION(BlueprintCallable, Category="Dash")
//	FVector CalculateFurthestValidDashDestination(const FVector& StartLocation, const FVector& Direction);
//
//protected:
//	/** Projects a location onto the ground surface using line tracing */
//	FVector ProjectLocationOntoGround(const FVector& Location, const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& QueryParams);
//#pragma endregion Dash Calculations
//
//#pragma region Blueprint Events
//public:
//	/** Virtual method for derived classes to handle dash trail effects */
//	UFUNCTION(BlueprintImplementableEvent, Category="Dash")
//	void OnDashStateChanged(bool bEnabled);
//#pragma endregion Blueprint Events
};