#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Abilities/TetheredGameplayAbility.h"
#include "UGA_DashAbility.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;

UENUM(BlueprintType)
enum class EDashDirectionSource: uint8
{
	OwnerForward    UMETA(DisplayName = "Owner Forward"),   // default
	MovementInput   UMETA(DisplayName = "Movement Input"),  // if none, fallback to forward
	AutoAim        UMETA(DisplayName = "Auto Aim"),       // if none, fallback to forward
	Custom         UMETA(DisplayName = "Custom (hook)"),  // BlueprintNativeEvent hook
};

/**
 * Blueprint-first dash ability with C++ plumbing.
 * - Endpoint sampling via UAbilityTask_DashQuery (furthest valid point)
 * - Motion Warping sync target feed
 * - Montage-driven, with early EndAbility on AnimNotify (StopWhenAbilityEnds=false)
 * - Exposes BlueprintNativeEvent hooks so BP or C++ variants can opt-in to extra behavior
 */
UCLASS(Blueprintable)
class TETHERED_API UGA_DashAbility : public UTetheredGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_DashAbility();

	// ============================================================================
	// Designer Configuration Properties (set per BP variant)
	// ============================================================================

	// Query Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (Units = "cm", ClampMin = "0"))
	float MaxDistance = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (ClampMin = "1", ClampMax = "60"))
	int32 NumSamples = 12;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query")
	bool bProjectToNav = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query")
	bool bSnapToGround = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (Units = "cm", ClampMin = "0"))
	float MaxHeightDelta = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query", meta = (Units = "cm", ClampMin = "0"))
	float ClearanceBuffer = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Query")
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Debug")
	float DebugSeconds = 0.f;

	// Motion Warping + Montage Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|MotionWarping")
	FName WarpSyncPoint = TEXT("DashTarget");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Montage")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Montage")
	FName MontageSection = TEXT("Dash");

	// Direction Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Movement")
	EDashDirectionSource DirectionSource = EDashDirectionSource::OwnerForward;

	// Collision Settings
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Collision")
	bool bUseDashGhostCollision = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Collision")
	TEnumAsByte<ECollisionChannel> DashGhostCollisionObjectType = ECC_GameTraceChannel1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Collision")
	bool bKeepVisibilityBlocking = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Movement")
	TEnumAsByte<EMovementMode> DashMovementMode = MOVE_Flying;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DashGravityScale = 0.f;


	// Early-end Control
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Handoff")
	FGameplayTag HandoffEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Handoff", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FallbackEarlyEndAtSectionTimeRatio = 0.0f;

	// Optional State Tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|Tags")
	FGameplayTag DashingStateTag;

	// Variant-driven attack window & input (binds only if tags set)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|AttackWindow")
	FGameplayTag AttackWindowOpenTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|AttackWindow")
	FGameplayTag AttackWindowCloseTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dash|AttackWindow")
	FGameplayTag DashAttackInputTag;

protected:
	// ============================================================================
	// Gameplay Ability System Overrides
	// ============================================================================
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	// ============================================================================
	// C++ Extension Points
	// ============================================================================
	virtual void OnDashTargetChosen(const FVector& EndLocation, const FRotator& Facing, float TravelDistance) {}

	// ============================================================================
	// Blueprint Native Event Hooks
	// ============================================================================
	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	FVector ComputeDesiredDirection() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnBeforePlayMontage(UPARAM(ref) UAnimMontage*& MontageToPlay, UPARAM(ref) FName& SectionName);

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	bool ShouldEndOnHandoff(const FGameplayEventData& Payload) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnDashStarted(FVector StartLocation, FVector ExpectedEndLocation);

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnDashEnded(bool bWasCancelled);

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnDashHandoff();

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnDashPassThrough(FVector StartLocation, FVector EndLocation);

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnDashAttackWindowOpened();

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnDashAttackWindowClosed();

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	bool OnDashAttackInput(const FGameplayEventData& Payload) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Dash|Hooks")
	void OnProjectileDeflected(AActor* Projectile);

private:
	// ============================================================================
	// C++ Extension Points (Virtual functions for C++ derived classes)
	// ============================================================================
	virtual bool GetCustomDirection(FVector& OutDirection) const { return false; }
	virtual bool GetAutoAimDirection(FVector& OutDirection) const { return false; }
	FVector DefaultDirectionComputation() const;

	// ============================================================================
	// Internal State
	// ============================================================================
	TWeakObjectPtr<class ACharacter> CachedCharacter;
	TWeakObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask;
	FVector ChosenEnd = FVector::ZeroVector;
	FRotator ChosenFacing = FRotator::ZeroRotator;
	FVector DashStartLoc = FVector::ZeroVector;
	bool bEndedAbilityEarly = false;
	bool bDashAttackWindowOpen = false;

	// Dash ghosting backup
	FName SavedCapsuleProfile = NAME_None;
	TEnumAsByte<ECollisionEnabled::Type> SavedCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	TEnumAsByte<EMovementMode> SavedMovementMode = MOVE_Walking;
	float SavedGravityScale = 1.f;
	bool bDashGhostActive = false;

	// ============================================================================
	// Internal Event Handlers
	// ============================================================================
	UFUNCTION()
	void OnDashQueryResult(bool bFound, FVector EndLocation, FRotator Facing, float Travel);

	void BindAttackWindowEvents();

	UFUNCTION()
	void OnHandoffEvent(struct FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnAttackWindowOpen(struct FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackWindowClose(struct FGameplayEventData Payload);

	UFUNCTION()
	void OnAttackInputEvent(struct FGameplayEventData Payload);

	// ============================================================================
	// Helper Functions
	// ============================================================================
	void SetupFallbackEarlyEndTimer();
	void EnableDashGhostCollision();
	void DisableDashGhostCollision();

	void ReadyDashMontage(FRotator Facing, FVector EndLocation);
	void SetupHandoffEvents();

};



