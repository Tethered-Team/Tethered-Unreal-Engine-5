// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/PlayerMovementComponent.h"
#include "Character/TetheredCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UPlayerMovementComponent::UPlayerMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UPlayerMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Update dash
	if (bIsDashing)
	{
		UpdateDash(DeltaTime);
	}
	
	// Process dash input buffer
	ProcessDashInputBuffer(DeltaTime);
	
	// Update speed overrides
	UpdateSpeedOverrides(DeltaTime);
	
	// Update movement state
	UpdateMovementState();
}

void UPlayerMovementComponent::Initialize(ATetheredCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
	
	if (OwnerCharacter)
	{
		CharacterMovement = OwnerCharacter->GetCharacterMovement();
		FollowCamera = OwnerCharacter->GetFollowCamera();
		
		// Configure initial movement settings
		if (CharacterMovement)
		{
			CharacterMovement->MaxWalkSpeed = BaseWalkSpeed;
			CharacterMovement->MaxAcceleration = MovementAcceleration;
			CharacterMovement->BrakingDecelerationWalking = MovementDeceleration;
		}
	}
}

bool UPlayerMovementComponent::IsMoving() const
{
	return CharacterMovement && CharacterMovement->Velocity.SizeSquared() > 1.0f;
}

#pragma region Input Handling
void UPlayerMovementComponent::HandleMovementInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	SetMovementInput(MovementVector.Y, MovementVector.X);
}

void UPlayerMovementComponent::SetMovementInput(float Forward, float Right)
{
	if (!OwnerCharacter || bIsDashing)
	{
		return;
	}
	
	CurrentMovementInput = FVector2D(Right, Forward);
	ProcessMovementInput(Forward, Right);
	
	// Call Blueprint event
	OnMovementInputReceived(CurrentMovementInput);
}

void UPlayerMovementComponent::ProcessMovementInput(float Forward, float Right)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Get camera-relative directions
	FVector ForwardDirection, RightDirection;
	GetCameraRelativeDirections(ForwardDirection, RightDirection);
	
	// Calculate movement direction
	FVector MovementDirection = (ForwardDirection * Forward) + (RightDirection * Right);
	
	// Store last movement direction for dash
	if (!MovementDirection.IsNearlyZero())
	{
		LastMovementDirection = MovementDirection.GetSafeNormal();
	}
	
	// Apply movement
	if (OwnerCharacter)
	{
		OwnerCharacter->AddMovementInput(MovementDirection, 1.0f);
		
		// Handle rotation
		if (bRotateToMovementDirection && !MovementDirection.IsNearlyZero())
		{
			UpdateCharacterRotation(MovementDirection, GetWorld()->GetDeltaSeconds());
		}
	}
}

void UPlayerMovementComponent::GetCameraRelativeDirections(FVector& OutForward, FVector& OutRight) const
{
	if (bUseCameraRelativeMovement && FollowCamera)
	{
		const FRotator CamRot = FollowCamera->GetComponentRotation();
		const FRotator YawRotation(0.f, CamRot.Yaw, 0.f);
		
		OutForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		OutRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}
	else if (OwnerCharacter)
	{
		const FRotator ControlRot = OwnerCharacter->GetControlRotation();
		const FRotator YawRotation(0.f, ControlRot.Yaw, 0.f);
		
		OutForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		OutRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}
	else
	{
		OutForward = FVector::ForwardVector;
		OutRight = FVector::RightVector;
	}
}

void UPlayerMovementComponent::UpdateCharacterRotation(const FVector& MovementDirection, float DeltaTime)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	const FRotator TargetRotation = MovementDirection.Rotation();
	const FRotator CurrentRotation = OwnerCharacter->GetActorRotation();
	
	float YawDifference = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw);
	float NewYaw = CurrentRotation.Yaw + (YawDifference * FMath::Clamp(DeltaTime * RotationInterpSpeed, 0.0f, 1.0f));
	
	OwnerCharacter->SetActorRotation(FRotator(0, NewYaw, 0));
}

void UPlayerMovementComponent::HandleDashInput()
{
	if (CanDash())
	{
		StartDash();
	}
	else if (!bIsDashing)
	{
		// Buffer the input
		bDashInputBuffered = true;
		DashInputBufferStartTime = GetWorld()->GetTimeSeconds();
	}
}

void UPlayerMovementComponent::SetRunState(bool bShouldRun)
{
	if (bIsRunning != bShouldRun)
	{
		bIsRunning = bShouldRun;
		UpdateMovementSpeed();
	}
}

void UPlayerMovementComponent::StopAllMovement()
{
	CurrentMovementInput = FVector2D::ZeroVector;
	
	if (bIsDashing)
	{
		EndDash();
	}
	
	if (CharacterMovement)
	{
		CharacterMovement->StopMovementImmediately();
	}
}
#pragma endregion Input Handling

#pragma region Dash System
void UPlayerMovementComponent::StartDash()
{
	if (!CanDash() || !CharacterMovement)
	{
		return;
	}
	
	bIsDashing = true;
	DashTimeRemaining = DashDuration;
	LastDashTime = GetWorld()->GetTimeSeconds();
	DashDirection = CalculateDashDirection();
	
	// Set dash velocity
	FVector DashVelocity = DashDirection * DashSpeed;
	CharacterMovement->Velocity = DashVelocity;
	CharacterMovement->SetMovementMode(MOVE_Flying);
	
	// Call Blueprint event
	OnDashStarted(DashDirection);
}

void UPlayerMovementComponent::UpdateDash(float DeltaTime)
{
	if (!bIsDashing || !CharacterMovement)
	{
		return;
	}
	
	DashTimeRemaining -= DeltaTime;
	
	if (DashTimeRemaining <= 0.0f)
	{
		EndDash();
		return;
	}
	
	// Maintain dash velocity
	FVector DashVelocity = DashDirection * DashSpeed;
	CharacterMovement->Velocity = DashVelocity;
}

void UPlayerMovementComponent::EndDash()
{
	if (!bIsDashing || !CharacterMovement)
	{
		return;
	}
	
	bIsDashing = false;
	DashTimeRemaining = 0.0f;
	
	// Return to walking movement
	CharacterMovement->SetMovementMode(MOVE_Walking);
	
	// Reduce velocity gradually for smooth transition
	FVector CurrentVelocity = CharacterMovement->Velocity;
	CurrentVelocity *= 0.3f;
	CharacterMovement->Velocity = CurrentVelocity;
	
	// Call Blueprint event
	OnDashEnded();
}

bool UPlayerMovementComponent::CanDash() const
{
	if (bIsDashing)
	{
		return false;
	}
	
	// Check cooldown
	float TimeSinceLastDash = GetWorld()->GetTimeSeconds() - LastDashTime;
	if (TimeSinceLastDash < DashCooldown)
	{
		return false;
	}
	
	return true;
}

FVector UPlayerMovementComponent::CalculateDashDirection() const
{
	if (bDashInMovementDirection && !LastMovementDirection.IsNearlyZero())
	{
		return LastMovementDirection;
	}
	
	// Default to forward direction
	if (OwnerCharacter)
	{
		return OwnerCharacter->GetActorForwardVector();
	}
	
	return FVector::ForwardVector;
}

void UPlayerMovementComponent::ProcessDashInputBuffer(float DeltaTime)
{
	if (bDashInputBuffered)
	{
		float TimeSinceBuffer = GetWorld()->GetTimeSeconds() - DashInputBufferStartTime;
		
		if (TimeSinceBuffer >= DashInputBufferTime)
		{
			bDashInputBuffered = false;
		}
		else if (CanDash())
		{
			bDashInputBuffered = false;
			StartDash();
		}
	}
}
#pragma endregion Dash System

#pragma region Speed Management
void UPlayerMovementComponent::SetMovementSpeedOverride(float SpeedMultiplier, float Duration)
{
	bHasSpeedOverride = true;
	SpeedOverrideMultiplier = SpeedMultiplier;
	
	if (Duration > 0.0f)
	{
		SpeedOverrideEndTime = GetWorld()->GetTimeSeconds() + Duration;
	}
	else
	{
		SpeedOverrideEndTime = -1.0f;
	}
	
	UpdateMovementSpeed();
}

void UPlayerMovementComponent::ClearMovementSpeedOverride()
{
	bHasSpeedOverride = false;
	SpeedOverrideMultiplier = 1.0f;
	SpeedOverrideEndTime = 0.0f;
	
	UpdateMovementSpeed();
}

float UPlayerMovementComponent::GetEffectiveMovementSpeed() const
{
	float BaseSpeed = bIsRunning ? BaseWalkSpeed * RunSpeedMultiplier : BaseWalkSpeed;
	return bHasSpeedOverride ? BaseSpeed * SpeedOverrideMultiplier : BaseSpeed;
}

void UPlayerMovementComponent::UpdateMovementSpeed()
{
	if (CharacterMovement && !bIsDashing)
	{
		CharacterMovement->MaxWalkSpeed = GetEffectiveMovementSpeed();
	}
}

void UPlayerMovementComponent::UpdateSpeedOverrides(float DeltaTime)
{
	if (bHasSpeedOverride && SpeedOverrideEndTime > 0.0f)
	{
		if (GetWorld()->GetTimeSeconds() >= SpeedOverrideEndTime)
		{
			ClearMovementSpeedOverride();
		}
	}
}
#pragma endregion Speed Management

#pragma region State Management
void UPlayerMovementComponent::UpdateMovementState()
{
	EPlayerMovementState NewState = CurrentMovementState;
	
	if (bIsDashing)
	{
		NewState = EPlayerMovementState::Dashing;
	}
	else if (CharacterMovement && CharacterMovement->IsFalling())
	{
		NewState = EPlayerMovementState::InAir;
	}
	else if (IsMoving())
	{
		NewState = bIsRunning ? EPlayerMovementState::Running : EPlayerMovementState::Walking;
	}
	else
	{
		NewState = EPlayerMovementState::Idle;
	}
	
	if (NewState != CurrentMovementState)
	{
		OnMovementStateChanged(CurrentMovementState, NewState);
		CurrentMovementState = NewState;
	}
}

void UPlayerMovementComponent::OnMovementStateChanged(EPlayerMovementState OldState, EPlayerMovementState NewState)
{
	// Handle any C++ logic for state changes
	
	// Call Blueprint event
	OnMovementStateChangedBP(OldState, NewState);
}
#pragma endregion State Management