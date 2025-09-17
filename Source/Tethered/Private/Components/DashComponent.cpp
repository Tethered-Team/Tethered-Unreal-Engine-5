// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/DashComponent.h"
#include "Character/TetheredCharacter.h"
#include "Components/EnhancedMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"

UDashComponent::UDashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	DashDistance = 500.0f;
	DashSpeed = 2000.0f;
	MinDashDistance = 100.0f;
	DashCollisionRadius = 40.0f;
	DashCollisionSteps = 10;
	DashToRunTransitionTime = 0.5f;
	DashMomentumRetention = 0.3f;
}

void UDashComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// If we're dashing, move towards the target location
	if (bIsDashing && OwnerCharacter)
	{
		const FVector CurrentLocation = OwnerCharacter->GetActorLocation();
		const FVector Direction = (DashTargetLocation - CurrentLocation).GetSafeNormal();
		const float DistanceToTarget = (DashTargetLocation - CurrentLocation).Size();
		
		const float DistanceThisFrame = DashSpeed * DeltaTime;
		
		if (DistanceThisFrame >= DistanceToTarget)
		{
			// We've reached or exceeded the target location
			OwnerCharacter->SetActorLocation(DashTargetLocation);
			bIsDashing = false;
			// End the dash montage if it's still playing
			if (DashMontage && OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_IsPlaying(DashMontage))
			{
				OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Stop(0.1f, DashMontage);
			}
			// Call the dash ended handler
			DashMontageEnded(DashMontage, true);
		}
		else
		{
			// Move towards the target location
			FVector NewLocation = CurrentLocation + (Direction * DistanceThisFrame);
			OwnerCharacter->SetActorLocation(NewLocation);
		}
	}
}

void UDashComponent::Initialize(ATetheredCharacter* InOwnerCharacter, UEnhancedMovementComponent* InMovementComponent)
{
	OwnerCharacter = InOwnerCharacter;
	MovementComponent = InMovementComponent;
	
	// Bind the dash montage ended delegate
	OnDashMontageEnded.BindUObject(this, &UDashComponent::DashMontageEnded);
}

void UDashComponent::DoDash()
{
	if (!OwnerCharacter || bIsDashing)
	{
		return;
	}
	
	// Get the dash direction based on input or forward vector
	FVector DashDirection = OwnerCharacter->GetActorForwardVector();
	
	// Calculate the furthest valid dash destination
	const FVector StartLocation = OwnerCharacter->GetActorLocation();
	DashTargetLocation = CalculateFurthestValidDashDestination(StartLocation, DashDirection);
	
	// If we didn't find a valid dash location far enough away, abort
	if ((DashTargetLocation - StartLocation).SizeSquared() < FMath::Square(MinDashDistance))
	{
		UE_LOG(LogTemp, Warning, TEXT("Dash cancelled: No valid landing spot found"));
		return;
	}
	
	// Raise the dash flags
	bIsDashing = true;
	bHasDashed = true;
	
	// Disable gravity while dashing
	if (UCharacterMovementComponent* CharMovement = OwnerCharacter->GetCharacterMovement())
	{
		CharMovement->GravityScale = 0.0f;
		
		// Calculate dash velocity based on distance and speed
		const FVector DashVector = DashTargetLocation - StartLocation;
		const FVector DashVelocity = DashVector.GetSafeNormal() * DashSpeed;
		
		// Set the dash velocity
		CharMovement->Velocity = DashVelocity;
	}
	
	// Log the dash for debugging
	UE_LOG(LogTemp, Log, TEXT("Dashing from %s to %s (Distance: %.1f)"),
		*StartLocation.ToString(), *DashTargetLocation.ToString(), (DashTargetLocation - StartLocation).Size());
	
	// Notify derived classes about dash state change
	OnDashStateChanged(true);
	
	// Play the dash montage
	if (DashMontage && OwnerCharacter->GetMesh())
	{
		if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			const float MontageLength = AnimInstance->Montage_Play(DashMontage, 2.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
			
			// Has the montage played successfully?
			if (MontageLength > 0.0f)
			{
				AnimInstance->Montage_SetEndDelegate(OnDashMontageEnded, DashMontage);
			}
		}
	}
}

void UDashComponent::EndDash()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Restore gravity (using default gravity scale of 1.0)
	if (UCharacterMovementComponent* CharMovement = OwnerCharacter->GetCharacterMovement())
	{
		CharMovement->GravityScale = 1.0f;
	}
	
	// Reset the dashing flag
	bIsDashing = false;
	
	// Clear the dash target
	DashTargetLocation = FVector::ZeroVector;
	
	// Are we grounded after the dash?
	if (OwnerCharacter->GetCharacterMovement()->IsMovingOnGround())
	{
		// Reset the dash usage flag, since we won't receive a landed event
		bHasDashed = false;
		
		// Notify derived classes about dash state change
		OnDashStateChanged(false);
	}
}

void UDashComponent::ResetDashState()
{
	bHasDashed = false;
	OnDashStateChanged(false);
}

void UDashComponent::DashMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// If the montage was interrupted, end the dash
	EndDash();
}

FVector UDashComponent::CalculateFurthestValidDashDestination(const FVector& StartLocation, const FVector& Direction)
{
	if (!OwnerCharacter)
	{
		return StartLocation;
	}
	
	// Calculate the ideal end location (horizontal projection)
	const FVector IdealEndLocation = StartLocation + (Direction * DashDistance);
	
	// Set up collision parameters for landing spot checks (only solid world geometry)
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bTraceComplex = false;
	
	// Only check against solid world geometry for landing spots
	FCollisionObjectQueryParams LandingObjectParams;
	LandingObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(DashCollisionRadius);
	
	// Track all valid landing positions
	TArray<FVector> ValidLandingSpots;
	
	// Test collision points along the entire dash path
	for (int32 Step = 1; Step <= DashCollisionSteps; ++Step)
	{
		const float Alpha = static_cast<float>(Step) / DashCollisionSteps;
		FVector TestLocation = FMath::Lerp(StartLocation, IdealEndLocation, Alpha);
		
		// Project the test location onto the ground surface
		FVector ProjectedLocation = ProjectLocationOntoGround(TestLocation, LandingObjectParams, QueryParams);
		
		// If projection failed (no ground found), use the original test location
		if (ProjectedLocation.Equals(FVector::ZeroVector))
		{
			ProjectedLocation = TestLocation;
		}
		
		// Check if this position is a valid landing spot
		FHitResult LandingHit;
		const bool bLandingBlocked = GetWorld()->SweepSingleByObjectType(
			LandingHit,
			ProjectedLocation,
			ProjectedLocation, // Same start/end for overlap test
			FQuat::Identity,
			LandingObjectParams,
			SphereShape,
			QueryParams);
		
		if (!bLandingBlocked)
		{
			ValidLandingSpots.Add(ProjectedLocation);
		}
	}
	
	// Find the furthest valid landing spot
	FVector FurthestValidSpot = StartLocation;
	float FurthestDistanceSq = 0.0f;
	
	for (const FVector& ValidSpot : ValidLandingSpots)
	{
		const float DistanceSq = (ValidSpot - StartLocation).SizeSquared();
		if (DistanceSq > FurthestDistanceSq && DistanceSq >= FMath::Square(MinDashDistance))
		{
			FurthestDistanceSq = DistanceSq;
			FurthestValidSpot = ValidSpot;
		}
	}
	
	// Log debug information
	UE_LOG(LogTemp, Log, TEXT("Found %d valid landing spots, furthest at distance %.1f"),
		ValidLandingSpots.Num(), FMath::Sqrt(FurthestDistanceSq));
	
	return FurthestValidSpot;
}

FVector UDashComponent::ProjectLocationOntoGround(const FVector& Location, const FCollisionObjectQueryParams& ObjectParams, const FCollisionQueryParams& QueryParams)
{
	if (!GetWorld())
	{
		return FVector::ZeroVector;
	}
	
	// Trace downward from above the location to find the ground
	const float TraceDistance = 10.0f; // Adjust based on your level geometry
	const FVector TraceStart = Location + FVector(0, 0, TraceDistance);
	const FVector TraceEnd = Location - FVector(0, 0, TraceDistance);
	
	FHitResult GroundHit;
	const bool bHitGround = GetWorld()->LineTraceSingleByObjectType(
		GroundHit,
		TraceStart,
		TraceEnd,
		ObjectParams,
		QueryParams);
	
	if (bHitGround && OwnerCharacter)
	{
		// Add a small offset above the ground to prevent clipping
		const float GroundOffset = OwnerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() + 5.0f;
		return GroundHit.ImpactPoint + FVector(0, 0, GroundOffset);
	}
	
	// If no ground found, return zero vector as a failure indicator
	return FVector::ZeroVector;
}