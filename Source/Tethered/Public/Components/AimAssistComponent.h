// AimAssistComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/AimAssistProfile.h"
#include "AimAssistComponent.generated.h"

class APlayerController;
class ACharacter;
class UProjectileMovementComponent;

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TETHERED_API UAimAssistComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAimAssistComponent();

	/** Sets the magnitude of aim input (0-1) for adjusting assist strength */
	void SetAimInputMagnitude(float Magnitude) { AimInputMagnitude = FMath::Clamp(Magnitude, 0.f, 1.f); }
	
	/** Sets the active aim assist profile containing all settings */
	UFUNCTION(BlueprintCallable) void SetActiveProfile(UAimAssistProfile* InProfile) { Profile = InProfile; }

	// Attack hooks
	/** Called when a melee attack is committed - applies turn assist and lunge */
	void OnMeleeCommit();
	
	/** Called when a ranged projectile is fired - applies steering and homing */
	void OnRangedFire(AActor* Projectile, UProjectileMovementComponent* OptionalProjMove);

	/** Returns the currently targeted actor, if any */
	UFUNCTION(BlueprintPure) AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	/** Global debug state for aim assist visualization - accessible to console commands */
	static bool bGlobalDebugEnabled;

protected:
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void TickComponent(float DeltaTime, ELevelTick, FActorComponentTickFunction*) override;

private:
	// Query
	/** Searches for potential targets within range and selects the best one */
	void QueryForTarget();
	
	/** Checks if target is within the field of view cone */
	bool PassesFOV2D(const FVector& ToTarget2D) const;
	
	/** Performs line of sight check to target's aim point */
	bool HasLineOfSightToAimPoint(AActor* Target) const;
	
	/** Calculates a score for target prioritization based on distance, angle, and velocity */
	float ScoreTarget(AActor* Target, float Dist2D, float Dot2D, bool bIsCurrent) const;

	// Assist
	/** Applies various forms of aim assistance each frame */
	void ApplyAssist(float Dt);
	
	/** Rotates the character toward the target by the specified degrees */
	void ApplyYawToward(const FVector2D& ToTarget2D, float Degrees);
	
	/** Calculates assist strength based on input magnitude (stronger when barely moving stick) */
	float AssistStrengthByStick() const;

	// Debug Visualization
	/** Draws debug information when global debug is enabled */
	void DrawDebugInfo();
	
	/** Draws the detection range sphere */
	void DrawDetectionRange();
	
	/** Draws the FOV cone */
	void DrawFOVCone();
	
	/** Draws assist cones (friction, snap) */
	void DrawAssistCones();
	
	/** Draws line of sight traces to targets */
	void DrawLineOfSightTraces();
	
	/** Draws target information and scores */
	void DrawTargetInfo();

	// Helpers
	/** Calculates 2D angle between two normalized vectors in degrees */
	static float Angle2D(const FVector2D& A, const FVector2D& B);
	
	/** Gets the player character's world position */
	FVector     GetPlayerPos() const;
	
	/** Gets the character's forward direction in 2D from yaw rotation (used for all targeting) */
	FVector2D   GetTargetingDirection2D() const;
	
	/** Gets the world position of target's aim point */
	FVector     GetAimPointWorld(AActor* Target) const;
	
	/** Gets the scene component designated as the aim point for a target */
	USceneComponent* GetAimPointComp(AActor* Target) const;

private:
	UPROPERTY(EditAnywhere, Category = "Profiles")
	TObjectPtr<UAimAssistProfile> Profile;

	// Overlap by object types (Pawn by default)
	UPROPERTY(EditAnywhere, Category = "Query") TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes =
	{ UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn) };

	UPROPERTY(EditAnywhere, Category = "Query") TEnumAsByte<ECollisionChannel> LOSChannel = ECC_Visibility;
	UPROPERTY(EditAnywhere, Category = "Query") float QueryInterval = 0.07f;

	FTimerHandle QueryTimer;

	UPROPERTY() TWeakObjectPtr<AActor> CurrentTarget;
	float     AimInputMagnitude = 0.f;

	// Score weights
	float WDist = 0.6f, WAngle = 1.0f, WVel = 0.2f, WSticky = 0.5f;

	// Cached
	UPROPERTY() TWeakObjectPtr<ACharacter> OwnerChar;
	UPROPERTY() TWeakObjectPtr<APlayerController> PC;
	
	// Cache potential targets for debug display
	TArray<AActor*> DebugPotentialTargets;
	TArray<float> DebugTargetScores;
};
