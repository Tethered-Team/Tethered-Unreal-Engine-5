// AbilityTask_DashQuery.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_DashQuery.generated.h"

/** Broadcast once with the sampled result. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
    FDashQueryResult,
    bool, bFound,
    FVector, EndLocation,
    FRotator, Facing,
    float, TravelDistance /*2D in cm*/);

/**
 * Samples candidate endpoints along a direction and returns the furthest valid one.
 * Valid = optional NavMesh projection -> optional ground snap -> capsule-overlap-free at end.
 * Fires once and ends immediately (synchronous).
 */
UCLASS()
class TETHERED_API UAbilityTask_DashQuery : public UAbilityTask
{
    GENERATED_BODY()
public:
    /** Result: fires once immediately after the query runs. */
    UPROPERTY(BlueprintAssignable, Category = "Tethered|Dash")
    FDashQueryResult OnResult;

    /** Single-channel version (backwards compatible). */
    UFUNCTION(BlueprintCallable, Category = "Tethered|Dash",
        meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility",
            BlueprintInternalUseOnly = "true", DisplayName = "Dash: Sample Endpoint"))
    static UAbilityTask_DashQuery* DashSampleEndpoint(
        UGameplayAbility* OwningAbility,
        float MaxDistance = 900.f,
        int32 NumSamples = 12,
        FVector DesiredDirection = FVector::ZeroVector,
        bool bProjectToNav = true,
        bool bSnapToGround = true,
        float MaxHeightDelta = 120.f,
        float ClearanceBuffer = 10.f,
        TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Pawn,
        float DebugSeconds = 0.f);

    /** Multi-channel version (BP-friendly array). */
    UFUNCTION(BlueprintCallable, Category = "Tethered|Dash",
        meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility",
            BlueprintInternalUseOnly = "true", DisplayName = "Dash: Sample Endpoint (Multi-Channel)"))
    static UAbilityTask_DashQuery* DashSampleEndpoint_MultiChannel(
        UGameplayAbility* OwningAbility,
        float MaxDistance,
        int32 NumSamples,
        FVector DesiredDirection,
        bool bProjectToNav,
        bool bSnapToGround,
        float MaxHeightDelta,
        float ClearanceBuffer,
        const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels,
        float DebugSeconds = 0.f);

    virtual void Activate() override;

private:
    bool RunQuery(class ACharacter* Character,
        FVector& OutLoc, FRotator& OutRot, float& OutTravel) const;

    // Params
    float P_MaxDistance = 900.f;
    int32 P_NumSamples = 12;
    FVector P_DesiredDir = FVector::ZeroVector;
    bool P_bProjectToNav = true;
    bool P_bSnapToGround = true;
    float P_MaxHeightDelta = 120.f;
    float P_Clearance = 10.f;
    /** If empty, defaults to { ECC_Pawn }. */
    TArray<TEnumAsByte<ECollisionChannel>> P_Channels;
    float P_DebugSeconds = 0.f;
};
