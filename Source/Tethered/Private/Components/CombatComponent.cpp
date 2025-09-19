// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/CombatComponent.h"
#include "Character/TetheredCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Interfaces/CombatDamageable.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

#if !UE_BUILD_SHIPPING
#include "Debug/TetheredCheatManager.h"
#endif

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Initialize default values
	MeleeDamage = 1.0f;
	MeleeTraceDistance = 150.0f;
	MeleeTraceRadius = 50.0f;
	MeleeKnockbackImpulse = 500.0f;
	MeleeLaunchImpulse = 300.0f;
	AttackInputCacheTimeTolerance = 0.5f;
	ComboInputCacheTimeTolerance = 1.0f;
	ChargeLoopSection = TEXT("ChargeLoop");
	ChargeAttackSection = TEXT("ChargeAttack");
	bDebugShowTraces = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::Initialize(ATetheredCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
	
	// Bind montage end delegates
	OnAttackMontageEnded.BindUObject(this, &UCombatComponent::AttackMontageEnded);
}

void UCombatComponent::DoComboAttackStart()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Are we already playing an attack animation?
	if (bIsAttacking)
	{
		// Cache the input time so we can check it later
		CachedAttackInputTime = GetWorld()->GetTimeSeconds();
		return;
	}
	
	// Perform a combo attack
	ComboAttack();
}

void UCombatComponent::DoComboAttackEnd()
{
	// Stub for now
}

void UCombatComponent::DoChargedAttackStart()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Raise the charging attack flag
	bIsChargingAttack = true;
	
	if (bIsAttacking)
	{
		// Cache the input time so we can check it later
		CachedAttackInputTime = GetWorld()->GetTimeSeconds();
		return;
	}
	
	ChargedAttack();
}

void UCombatComponent::DoChargedAttackEnd()
{
	// Lower the charging attack flag
	bIsChargingAttack = false;
	
	// If we've done the charge loop at least once, release the charged attack right away
	if (bHasLoopedChargedAttack)
	{
		CheckChargedAttack();
	}
}

void UCombatComponent::ComboAttack()
{
	if (!OwnerCharacter || !ComboAttackMontage)
	{
		return;
	}
	
	// Raise the attacking flag
	bIsAttacking = true;
	
	// Reset the combo count
	ComboCount = 0;
	
	// Play the attack montage
	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ComboAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		
		// Subscribe to montage completed and interrupted events
		if (MontageLength > 0.0f)
		{
			// Set the end delegate for the montage
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ComboAttackMontage);
		}
	}


}

void UCombatComponent::ChargedAttack()
{
	if (!OwnerCharacter || !ChargedAttackMontage)
	{
		return;
	}
	
	// Raise the attacking flag
	bIsAttacking = true;
	
	// Reset the charge loop flag
	bHasLoopedChargedAttack = false;
	
	// Play the charged attack montage
	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		
		// Subscribe to montage completed and interrupted events
		if (MontageLength > 0.0f)
		{
			// Set the end delegate for the montage
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ChargedAttackMontage);
		}
	}
}

void UCombatComponent::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// Reset the attacking flag
	bIsAttacking = false;
	
	// Check if we have a non-stale cached input
	if (GetWorld() && GetWorld()->GetTimeSeconds() - CachedAttackInputTime <= AttackInputCacheTimeTolerance)
	{
		// Are we holding the charged attack button?
		if (bIsChargingAttack)
		{
			// Do a charged attack
			ChargedAttack();
		}
		else
		{
			// Do a regular attack
			ComboAttack();
		}
	}
}

void UCombatComponent::DoAttackTrace(FName DamageSourceBone)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Sweep for objects in front of the character to be hit by the attack
	TArray<FHitResult> OutHits;
	
	// Start at the provided socket location, sweep forward
	const FVector TraceStart = OwnerCharacter->GetMesh()->GetSocketLocation(DamageSourceBone);
	const FVector TraceEnd = TraceStart + (OwnerCharacter->GetActorForwardVector() * MeleeTraceDistance);
	
	// Check for pawn and world dynamic collision object types
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	
	// Use a sphere shape for the sweep
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(MeleeTraceRadius);
	
	// Ignore self
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	// Check if debug visualization is enabled via the global combat debug state
	const bool bShouldShowDebug = bDebugShowTraces || IsGlobalCombatDebugEnabled();
	
	// Add debug visualization
	if (bShouldShowDebug && GetWorld())
	{
		// Draw the sweep as a series of spheres along the path
		const int32 NumSpheres = 5;
		for (int32 i = 0; i <= NumSpheres; ++i)
		{
			const float Alpha = static_cast<float>(i) / NumSpheres;
			const FVector SphereCenter = FMath::Lerp(TraceStart, TraceEnd, Alpha);
			
			// Green for start, red for end, yellow for middle
			FColor SphereColor = FColor::Yellow;
			if (i == 0) SphereColor = FColor::Green;
			else if (i == NumSpheres) SphereColor = FColor::Red;
			
			DrawDebugSphere(GetWorld(), SphereCenter, MeleeTraceRadius, 12, SphereColor, false, 2.0f, 0, 1.0f);
		}
		
		// Draw a line from start to end
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Cyan, false, 2.0f, 0, 2.0f);
		
		// Draw direction arrow
		DrawDebugDirectionalArrow(GetWorld(), TraceStart, TraceEnd, 50.0f, FColor::Blue, false, 2.0f, 0, 3.0f);
		
		// Draw trace info text
		DrawDebugString(GetWorld(), TraceStart + FVector(0, 0, 100), 
			FString::Printf(TEXT("Combat Trace: %.1fcm x %.1fcm"), MeleeTraceDistance, MeleeTraceRadius * 2.0f), 
			nullptr, FColor::White, 2.0f);
	}
	
	if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams))
	{
		// Iterate over each object hit
		for (const FHitResult& CurrentHit : OutHits)
		{
			// Debug visualization for hits
			if (bShouldShowDebug && GetWorld())
			{
				// Draw impact point and normal
				DrawDebugSphere(GetWorld(), CurrentHit.ImpactPoint, 10.0f, 8, FColor::Orange, false, 3.0f, 0, 2.0f);
				DrawDebugLine(GetWorld(), CurrentHit.ImpactPoint, CurrentHit.ImpactPoint + (CurrentHit.ImpactNormal * 100.0f), FColor::White, false, 3.0f, 0, 3.0f);
				
				// Draw text with actor name and damage info
				if (CurrentHit.GetActor())
				{
					DrawDebugString(GetWorld(), CurrentHit.ImpactPoint + FVector(0, 0, 50), 
						FString::Printf(TEXT("HIT: %s (%.1f dmg)"), *CurrentHit.GetActor()->GetName(), MeleeDamage), 
						nullptr, FColor::Red, 3.0f, true);
				}
			}
			
			// Check if we've hit a damageable actor
			if (ICombatDamageable* Damageable = Cast<ICombatDamageable>(CurrentHit.GetActor()))
			{
				// Knock upwards and away from the impact normal
				const FVector Impulse = (CurrentHit.ImpactNormal * -MeleeKnockbackImpulse) + (FVector::UpVector * MeleeLaunchImpulse);
				
				// Pass the damage event to the actor
				Damageable->ApplyDamage(MeleeDamage, OwnerCharacter, CurrentHit.ImpactPoint, Impulse);
				
				// Call the Blueprint event on the character
				if (OwnerCharacter)
				{
					OwnerCharacter->OnDealtDamage(MeleeDamage, CurrentHit.ImpactPoint);
				}
			}
		}
	}
	else
	{
		// Debug visualization when no hits
		if (bShouldShowDebug && GetWorld())
		{
			// Draw the full sweep in a dimmer color to show it didn't hit anything
			DrawDebugCapsule(GetWorld(), (TraceStart + TraceEnd) * 0.5f, MeleeTraceDistance * 0.5f + MeleeTraceRadius, MeleeTraceRadius, 
				FQuat::FindBetweenNormals(FVector::UpVector, OwnerCharacter->GetActorForwardVector()), 
				FColor::Red, false, 2.0f, 0, 1.0f);
			
			DrawDebugString(GetWorld(), TraceStart + FVector(0, 0, 50), TEXT("NO HITS"), nullptr, FColor::Red, 2.0f);
		}
	}
}

void UCombatComponent::CheckCombo()
{
	if (!OwnerCharacter)
	{
		return;

	}
	
	// Are we playing a non-charge attack animation?
	if (bIsAttacking && !bIsChargingAttack)
	{
		// Is the last attack input not stale?
		if (GetWorld() && GetWorld()->GetTimeSeconds() - CachedAttackInputTime <= ComboInputCacheTimeTolerance)
		{
			// Consume the attack input so we don't accidentally trigger it twice
			CachedAttackInputTime = 0.0f;
			
			// Increase the combo counter
			++ComboCount;
			
			// Do we still have a combo section to play?
			if (ComboCount < ComboSectionNames.Num())
			{
				// Jump to the next combo section
				if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
				{
					AnimInstance->Montage_JumpToSection(ComboSectionNames[ComboCount], ComboAttackMontage);
				}
			}
		}
	}
}

void UCombatComponent::CheckChargedAttack()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Raise the looped charged attack flag
	bHasLoopedChargedAttack = true;
	
	// Jump to either the loop or the attack section depending on whether we're still holding the charge button
	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(bIsChargingAttack ? ChargeLoopSection : ChargeAttackSection, ChargedAttackMontage);
	}
}

bool UCombatComponent::IsGlobalCombatDebugEnabled()
{
	// Check if the global combat debug is enabled via CheatManager
	// We'll access this through the CheatManager system
	#if !UE_BUILD_SHIPPING
		// Import the CheatManager header and use its static function
		return UTetheredCheatManager::IsGlobalCombatDebugEnabled();
	#else
		return false;
	#endif
}