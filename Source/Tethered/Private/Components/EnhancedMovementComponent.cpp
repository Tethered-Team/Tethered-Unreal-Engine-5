// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/EnhancedMovementComponent.h"
#include "Character/TetheredCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"

UEnhancedMovementComponent::UEnhancedMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UEnhancedMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEnhancedMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Handle smooth movement speed transitions
	UpdateMovementSpeedTransition(DeltaTime);
}

void UEnhancedMovementComponent::Initialize(ATetheredCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
	
	if (OwnerCharacter)
	{
		CharacterMovement = OwnerCharacter->GetCharacterMovement();
		
		if (CharacterMovement)
		{
			// Set initial movement properties
			CharacterMovement->MaxWalkSpeed = MaxWalkSpeed;
			CurrentMovementSpeed = MaxWalkSpeed;
			TargetMovementSpeed = MaxWalkSpeed;
		}
	}
}

#pragma region Movement Actions
void UEnhancedMovementComponent::DoMove(float Right, float Forward)
{
	ApplyEnhancedMovementInput(Forward, Right);
}

void UEnhancedMovementComponent::ApplyEnhancedMovementInput(float Forward, float Right)
{
	if (!OwnerCharacter || !CharacterMovement)
	{
		return;
	}
	
	// Get camera-relative directions
	FVector ForwardDirection = GetForwardDirection();
	FVector RightDirection = GetRightDirection();
	
	// Apply movement input
	OwnerCharacter->AddMovementInput(ForwardDirection, Forward);
	OwnerCharacter->AddMovementInput(RightDirection, Right);
}

FVector UEnhancedMovementComponent::GetForwardDirection() const
{
	if (!OwnerCharacter)
	{
		return FVector::ForwardVector;
	}
	
	// Use camera direction if available
	if (UCameraComponent* Camera = OwnerCharacter->GetFollowCamera())
	{
		const FRotator CamRot = Camera->GetComponentRotation();
		const FRotator YawRotation(0.f, CamRot.Yaw, 0.f);
		return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	}
	
	// Fall back to controller rotation
	const FRotator ControlRot = OwnerCharacter->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRot.Yaw, 0.f);
	return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
}

FVector UEnhancedMovementComponent::GetRightDirection() const
{
	if (!OwnerCharacter)
	{
		return FVector::RightVector;
	}
	
	// Use camera direction if available
	if (UCameraComponent* Camera = OwnerCharacter->GetFollowCamera())
	{
		const FRotator CamRot = Camera->GetComponentRotation();
		const FRotator YawRotation(0.f, CamRot.Yaw, 0.f);
		return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	}
	
	// Fall back to controller rotation
	const FRotator ControlRot = OwnerCharacter->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRot.Yaw, 0.f);
	return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
}

void UEnhancedMovementComponent::SetRunState(bool bShouldRun)
{
	bRunHeld = bShouldRun;
	UpdateMovementSpeed();
}

void UEnhancedMovementComponent::UpdateMovementSpeed()
{
	if (!CharacterMovement)
	{
		return;
	}
	
	// Don't update speed if we're in a transition
	if (bIsInDashTransition)
	{
		TargetMovementSpeed = bRunHeld ? MaxRunSpeed : MaxWalkSpeed;
		return;
	}
	
	// Set the appropriate movement speed
	const float DesiredSpeed = bRunHeld ? MaxRunSpeed : MaxWalkSpeed;
	CharacterMovement->MaxWalkSpeed = DesiredSpeed;
	CurrentMovementSpeed = DesiredSpeed;
	TargetMovementSpeed = DesiredSpeed;
}

void UEnhancedMovementComponent::SetMovementSpeed(float NewSpeed)
{
	if (CharacterMovement)
	{
		CharacterMovement->MaxWalkSpeed = NewSpeed;
		CurrentMovementSpeed = NewSpeed;
		
		// If not in transition, also update target
		if (!bIsInDashTransition)
		{
			TargetMovementSpeed = NewSpeed;
		}
	}
}

void UEnhancedMovementComponent::StartDashToMovementTransition(float DashSpeed)
{
	if (!CharacterMovement)
	{
		return;
	}
	
	// Determine target speed based on current input state
	TargetMovementSpeed = bRunHeld ? MaxRunSpeed : MaxWalkSpeed;
	
	// Calculate starting speed based on current dash momentum
	const float DashEndSpeed = DashSpeed * DashMomentumRetention;
	
	// Start from a speed that's between dash speed and normal speed
	CurrentMovementSpeed = FMath::Max(DashEndSpeed, TargetMovementSpeed);
	
	// Set the initial high speed
	CharacterMovement->MaxWalkSpeed = CurrentMovementSpeed;
	
	// Enable transition flag
	bIsInDashTransition = true;
	
	// Apply some forward momentum to maintain fluid movement
	if (OwnerCharacter)
	{
		const FVector ForwardMomentum = OwnerCharacter->GetActorForwardVector() * (CurrentMovementSpeed * 0.3f);
		CharacterMovement->AddImpulse(ForwardMomentum, true);
	}
}

void UEnhancedMovementComponent::StopDashTransition()
{
	bIsInDashTransition = false;
	UpdateMovementSpeed();
}

void UEnhancedMovementComponent::UpdateMovementSpeedTransition(float DeltaTime)
{
	if (!bIsInDashTransition || !CharacterMovement)
	{
		return;
	}
	
	// Smoothly interpolate between current and target movement speed
	const float InterpSpeed = 1.0f / DashToRunTransitionTime;
	CurrentMovementSpeed = FMath::FInterpTo(CurrentMovementSpeed, TargetMovementSpeed, DeltaTime, InterpSpeed);
	
	// Update the character movement component
	CharacterMovement->MaxWalkSpeed = CurrentMovementSpeed;
	
	// Check if we've reached the target speed (within a small threshold)
	if (FMath::IsNearlyEqual(CurrentMovementSpeed, TargetMovementSpeed, 10.0f))
	{
		bIsInDashTransition = false;
		CurrentMovementSpeed = TargetMovementSpeed;
		CharacterMovement->MaxWalkSpeed = TargetMovementSpeed;
	}
}
#pragma endregion Movement Actions

#pragma region Enhanced Movement Features
bool UEnhancedMovementComponent::IsMoving() const
{
	if (!CharacterMovement)
	{
		return false;
	}
	
	return CharacterMovement->Velocity.SizeSquared() > FMath::Square(10.0f); // Moving faster than 10 cm/s
}

FVector UEnhancedMovementComponent::GetMovementDirection() const
{
	if (!CharacterMovement)
	{
		return FVector::ZeroVector;
	}
	
	FVector Velocity = CharacterMovement->Velocity;
	Velocity.Z = 0.0f; // Ignore vertical movement
	return Velocity.GetSafeNormal();
}
#pragma endregion Enhanced Movement Features