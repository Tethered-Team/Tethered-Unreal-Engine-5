// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/CombatDamageable.h"
#include "Interfaces/Aimable.h"
#include "CombatDummy.generated.h"

class UStaticMeshComponent;
class UPhysicsConstraintComponent;

/**
 *  A simple invincible combat training dummy
 */
UCLASS(abstract)
class ACombatDummy : public AActor, public ICombatDamageable, public IAimable
{
	GENERATED_BODY()
	
	/** Root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	/** Static base plate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BasePlate;

	/** Physics enabled dummy mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Dummy;

	/** Physics constraint holding the dummy and base plate together */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UPhysicsConstraintComponent* PhysicsConstraint;

public:	
	
	/** Constructor */
	ACombatDummy();

	// ~Begin CombatDamageable interface

		/** Handles damage and knockback events */
	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;

	/** Handles death events */
	virtual void HandleDeath() override;

	/** Handles healing events */
	virtual void ApplyHealing(float Healing, AActor* Healer) override;

	// ~End CombatDamageable interface

	// ~Begin IAimable interface
	
	/** Returns the component to aim at (head/torso). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Aimable")
	USceneComponent* GetAimPointComponent() const;
	virtual USceneComponent* GetAimPointComponent_Implementation() const;

	/** Returns whether this actor can be targeted right now. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Aimable")
	bool CanBeTargeted() const;
	virtual bool CanBeTargeted_Implementation() const;

	// ~End IAimable interface

protected:

	/** Blueprint handle to apply damage effects */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat", meta = (DisplayName = "On Dummy Damaged"))
	void BP_OnDummyDamaged(const FVector& Location, const FVector& Direction);
};
