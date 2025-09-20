// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HealthComponent.h"
#include "Character/TetheredCharacter.h"
#include "UI/CombatLifeBar.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Set default values
	MaxHP = 5.0f;
	CurrentHP = 0.0f;
	RespawnTime = 3.0f;
	DeathCameraDistance = 400.0f;
	LifeBarColor = FLinearColor::Red;
	PelvisBoneName = TEXT("pelvis");
	
	OwnerCharacter = nullptr;
	LifeBarWidget = nullptr;
	bIgnoreDamage = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Reset HP to maximum at start of play
	ResetHP();
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear any active respawn timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UHealthComponent::Initialize(ATetheredCharacter* InOwnerCharacter, UCombatLifeBar* InLifeBarWidget)
{
	OwnerCharacter = InOwnerCharacter;
	LifeBarWidget = InLifeBarWidget;
	
	// Set up the life bar widget if available
	if (LifeBarWidget)
	{
		LifeBarWidget->SetBarColor(LifeBarColor);
		UpdateLifeBarUI();
	}
}

void UHealthComponent::ResetHP()
{
	CurrentHP = MaxHP;
	bIgnoreDamage = false;
	UpdateLifeBarUI();
}

void UHealthComponent::ModifyHealth(float Amount, AActor* Instigator)
{
	if (bIgnoreDamage && Amount < 0.0f)
	{
		return;
	}
	
	float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.0f, MaxHP);
	
	// Update UI
	UpdateLifeBarUI();
	
	// Handle death if HP reaches zero
	if (CurrentHP <= 0.0f && OldHP > 0.0f)
	{
		HandleDeath();
	}
}

void UHealthComponent::RespawnCharacter()
{
	if (OwnerCharacter)
	{
		// Reset mesh physics
		if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
		{
			Mesh->SetPhysicsBlendWeight(0.0f);
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			
			// Reset mesh transform
			Mesh->SetRelativeTransform(OwnerCharacter->GetMeshStartingTransform());
		}
		
		// Reset camera distance
		if (USpringArmComponent* CameraBoom = OwnerCharacter->GetCameraBoom())
		{
			CameraBoom->TargetArmLength = OwnerCharacter->GetDefaultCameraDistance();
		}
		
		// Reset health
		ResetHP();
		
		// Call Blueprint event on character
		if (OwnerCharacter)
		{
			OwnerCharacter->OnRespawn();
		}
	}
}

void UHealthComponent::UpdateLifeBarUI()
{
	if (LifeBarWidget)
	{
		LifeBarWidget->SetLifePercentage(GetHealthPercentage());
	}
}

void UHealthComponent::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	if (bIgnoreDamage || Damage <= 0.0f)
	{
		return;
	}
	
	// Apply the damage
	ModifyHealth(-Damage, DamageCauser);
	
	// Apply physics impulse if character is alive and has a valid mesh
	if (CurrentHP > 0.0f && OwnerCharacter)
	{
		if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
		{
			if (Mesh->IsSimulatingPhysics())
			{
				// apply an impulse to the ragdoll
				Mesh->AddImpulseAtLocation(DamageImpulse * Mesh->GetMass(), DamageLocation);
			}
		}
	}
}

void UHealthComponent::ApplyHealing(float Healing, AActor* Healer)
{
	if (Healing <= 0.0f)
	{
		return;
	}
	
	ModifyHealth(Healing, Healer);
}

void UHealthComponent::HandleDeath()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	// Set ignore damage to prevent multiple death events
	bIgnoreDamage = true;
	
	// Enable ragdoll physics
	if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		Mesh->SetPhysicsBlendWeight(1.0f);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		
		// Add impulse to pelvis for ragdoll effect
		if (!PelvisBoneName.IsNone())
		{
			Mesh->AddImpulse(FVector(0.0f, 0.0f, 300.0f), PelvisBoneName);
		}
	}
	
	// Adjust camera distance for death
	if (USpringArmComponent* CameraBoom = OwnerCharacter->GetCameraBoom())
	{
		CameraBoom->TargetArmLength = DeathCameraDistance;
	}
	
	// Start respawn timer
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RespawnTimer,
			this,
			&UHealthComponent::RespawnCharacter,
			RespawnTime,
			false
		);
	}
}