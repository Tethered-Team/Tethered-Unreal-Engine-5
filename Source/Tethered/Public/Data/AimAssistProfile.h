// AimAssistProfile.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AimAssistProfile.generated.h"

/**
 * Data asset that defines aim assist behavior parameters.
 * Create instances of this asset to configure different aim assist profiles for various scenarios.
 */
UCLASS(BlueprintType)
class TETHERED_API UAimAssistProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// Query Settings - Controls target detection and selection
	
	/** Maximum distance to search for targets in centimeters. Larger values detect enemies further away but may impact performance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Query", 
		meta = (ClampMin = "100", ClampMax = "5000", Units = "cm", 
		ToolTip = "How far to look for targetable enemies. Recommended: 1500-2500cm for melee, 3000-5000cm for ranged combat."))
	float AssistRangeCm = 1800.f;

	/** Field of view cone in degrees for target detection. Smaller values require more precise aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Query", 
		meta = (ClampMin = "5", ClampMax = "180", Units = "deg",
		ToolTip = "Angle of the detection cone in front of the player. Recommended: 30-60 degrees. Larger values make targeting easier but less precise."))
	float QueryFOVDeg = 45.f;

	/** How much the current target 'sticks' to prevent jittery target switching. Higher values = less switching. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Query", 
		meta = (ClampMin = "0", ClampMax = "1",
		ToolTip = "Target stickiness factor (0-1). Higher values make it harder to switch targets, preventing jitter. Recommended: 0.4-0.8."))
	float Stickiness = 0.6f;

	// Assist Settings - Controls how aim assistance behaves
	
	/** Angle in degrees within which aim friction is applied to slow down crosshair movement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assist", 
		meta = (ClampMin = "1", ClampMax = "30", Units = "deg",
		ToolTip = "Cone angle for aim friction effect. When aiming near targets within this cone, camera sensitivity should be reduced. Recommended: 6-12 degrees."))
	float FrictionConeDeg = 8.f;

	/** Multiplier for aim sensitivity when friction is active. Lower values = more friction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assist", 
		meta = (ClampMin = "0.1", ClampMax = "1",
		ToolTip = "Sensitivity multiplier during friction (0.1-1). Lower values create stronger 'sticky' feeling when aiming at targets. For camera/look system use. Recommended: 0.5-0.8."))
	float FrictionScale = 0.7f;

	/** NOTE: Magnetism is not used when targeting is based on character forward direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assist|Unused", 
		meta = (ClampMin = "1", ClampMax = "20", Units = "deg",
		ToolTip = "UNUSED: Magnetism is disabled when using character-forward-only targeting."))
	float MagnetismConeDeg = 6.f;

	/** NOTE: Magnetism is not used when targeting is based on character forward direction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assist|Unused", 
		meta = (ClampMin = "0.05", ClampMax = "2",
		ToolTip = "UNUSED: Magnetism is disabled when using character-forward-only targeting."))
	float MagnetismStrength = 0.25f;

	/** Angle in degrees within which snap assistance activates (primarily for controllers/touch). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assist", 
		meta = (ClampMin = "1", ClampMax = "15", Units = "deg",
		ToolTip = "Cone angle for snap assistance on controllers/touch. Instantly snaps aim to targets within this very small cone. Recommended: 2-6 degrees."))
	float SnapConeDeg = 4.f;

	/** Maximum degrees of instant snap correction that can be applied. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assist", 
		meta = (ClampMin = "0.5", ClampMax = "10", Units = "deg",
		ToolTip = "Maximum instant snap correction in degrees. Prevents jarring large snaps. Should be smaller than SnapConeDeg. Recommended: 1-5 degrees."))
	float MaxSnapDeg = 3.f;

	/** Maximum rotation speed in degrees per second for rotational aim assistance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Assist", 
		meta = (ClampMin = "30", ClampMax = "500", Units = "deg/s",
		ToolTip = "Maximum speed of automatic rotation toward targets. Higher values allow faster target acquisition but may feel less natural. Recommended: 120-200 deg/s."))
	float MaxYawDegPerSec = 160.f;

	// Ranged Combat Settings - For projectile weapons
	
	/** Maximum degrees a projectile can be steered toward the target upon firing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged", 
		meta = (ClampMin = "0", ClampMax = "15", Units = "deg",
		ToolTip = "How much projectiles are steered toward targets when fired. Subtle values feel more natural. Recommended: 1-5 degrees."))
	float ProjectileSteerDeg = 2.f;

	/** Duration in seconds that projectiles will home toward targets after firing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranged", 
		meta = (ClampMin = "0", ClampMax = "1", Units = "s",
		ToolTip = "How long projectiles actively seek targets after firing. Short durations feel more skill-based. Recommended: 0.05-0.2 seconds."))
	float InitialHomingTime = 0.12f;

	// Melee Combat Settings - For close-range attacks
	
	/** Maximum distance in centimeters a melee lunge will cover to reach targets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee", 
		meta = (ClampMin = "50", ClampMax = "800", Units = "cm",
		ToolTip = "Maximum lunge distance for melee attacks. Helps close gaps to targets. Too high values may feel like teleporting. Recommended: 200-400cm."))
	float MeleeLungeDistCm = 300.f;

	/** Speed of the lunge movement in centimeters per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee", 
		meta = (ClampMin = "400", ClampMax = "3000", Units = "cm/s",
		ToolTip = "Speed of melee lunge movement. Higher values create faster, more aggressive lunges. Recommended: 800-1500 cm/s."))
	float MeleeLungeSpeed = 1200.f;

	/** Maximum degrees the character can turn during a melee attack to face the target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Melee", 
		meta = (ClampMin = "0", ClampMax = "90", Units = "deg",
		ToolTip = "Maximum auto-turn during melee attacks. Helps ensure attacks connect with nearby targets. Too high values may feel disorienting. Recommended: 15-45 degrees."))
	float MeleeMaxTurnOnAttack = 30.f;
};