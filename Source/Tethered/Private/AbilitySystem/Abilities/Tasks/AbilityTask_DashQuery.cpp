// AbilityTask_DashQuery.cpp

#include "AbilitySystem/Abilities/Tasks/AbilityTask_DashQuery.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h" // for ECC_*

UAbilityTask_DashQuery* UAbilityTask_DashQuery::DashSampleEndpoint(
    UGameplayAbility* OwningAbility,
    float MaxDistance,
    int32 NumSamples,
    FVector DesiredDirection,
    bool bProjectToNav,
    bool bSnapToGround,
    float MaxHeightDelta,
    float ClearanceBuffer,
    TEnumAsByte<ECollisionChannel> CollisionChannel,
    float DebugSeconds)
{
    // Forward to the multi-channel version with one element
    TArray<TEnumAsByte<ECollisionChannel>> Channels;
    Channels.Add(CollisionChannel);

    return DashSampleEndpoint_MultiChannel(
        OwningAbility,
        MaxDistance,
        NumSamples,
        DesiredDirection,
        bProjectToNav,
        bSnapToGround,
        MaxHeightDelta,
        ClearanceBuffer,
        Channels,
        DebugSeconds);
}

UAbilityTask_DashQuery* UAbilityTask_DashQuery::DashSampleEndpoint_MultiChannel(
    UGameplayAbility* OwningAbility,
    float MaxDistance,
    int32 NumSamples,
    FVector DesiredDirection,
    bool bProjectToNav,
    bool bSnapToGround,
    float MaxHeightDelta,
    float ClearanceBuffer,
    const TArray<TEnumAsByte<ECollisionChannel>>& CollisionChannels,
    float DebugSeconds)
{
    UAbilityTask_DashQuery* Task = NewAbilityTask<UAbilityTask_DashQuery>(OwningAbility);
    Task->P_MaxDistance = FMath::Max(0.f, MaxDistance);
    Task->P_NumSamples = FMath::Clamp(NumSamples, 1, 60);
    Task->P_DesiredDir = DesiredDirection;
    Task->P_bProjectToNav = bProjectToNav;
    Task->P_bSnapToGround = bSnapToGround;
    Task->P_MaxHeightDelta = FMath::Max(0.f, MaxHeightDelta);
    Task->P_Clearance = FMath::Max(0.f, ClearanceBuffer);
    Task->P_DebugSeconds = FMath::Max(0.f, DebugSeconds);
    Task->P_Channels = CollisionChannels;
    return Task;
}

void UAbilityTask_DashQuery::Activate()
{
    ACharacter* Char = Cast<ACharacter>(GetAvatarActor());
    FVector End; FRotator Rot; float Travel = 0.f;

    const bool bFound = RunQuery(Char, End, Rot, Travel);
    OnResult.Broadcast(bFound, End, Rot, Travel);
    EndTask(); // one-shot
}

bool UAbilityTask_DashQuery::RunQuery(ACharacter* Character,
    FVector& OutLoc, FRotator& OutRot, float& OutTravel) const
{
    if (!Character) return false;

    const UCapsuleComponent* Cap = Character->GetCapsuleComponent();
    if (!Cap) return false;

    UWorld* W = Character->GetWorld();
    if (!W) return false;

    const FVector Start = Character->GetActorLocation();

    // Direction 2D
    FVector Dir = P_DesiredDir.IsNearlyZero()
        ? Character->GetActorForwardVector()
        : P_DesiredDir;
    Dir = Dir.GetSafeNormal2D();
    if (Dir.IsNearlyZero()) return false;

    const float Rad = Cap->GetScaledCapsuleRadius() + P_Clearance;
    const float Half = Cap->GetScaledCapsuleHalfHeight();

    UNavigationSystemV1* Nav = P_bProjectToNav ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(W) : nullptr;

    FCollisionQueryParams QParams(TEXT("DashOverlap"), false, Character);
    QParams.AddIgnoredActor(Character);

    // Ensure at least one channel; default to Pawn
    TArray<TEnumAsByte<ECollisionChannel>> Channels = P_Channels;
    if (Channels.Num() == 0)
    {
        Channels.Add(ECC_Pawn);
    }

    auto ValidateCandidate = [&](const FVector& CandIn)->TOptional<FVector>
        {
            // 1) Base point (after optional nav projection)
            FVector BasePoint = CandIn;
            if (Nav)
            {
                FNavLocation Projected;
                if (!Nav->ProjectPointToNavigation(BasePoint, Projected)) return {};
                BasePoint = Projected.Location;
            }

            // 2) Compute the *capsule center* we'll test
            FVector Center = BasePoint;
            if (P_bSnapToGround)
            {
                FHitResult DownHit;
                const FVector S = BasePoint + FVector(0, 0, Half + 15.f);
                const FVector E = BasePoint - FVector(0, 0, Half + 200.f);
                if (W->LineTraceSingleByChannel(DownHit, S, E, ECollisionChannel::ECC_Visibility))
                {
                    constexpr float GroundClear = 1.0f; // tiny lift to avoid floor Z-fighting
                    Center = DownHit.Location + FVector(0, 0, Half + GroundClear);
                }
                // else: keep Center as BasePoint (no ground found)
            }
            else
            {
                // Keep capsule on the same Z plane as the start (character capsule center)
                Center.Z = Start.Z;
            }

            // 3) Height guard vs start plane (after snap)
            if (FMath::Abs(Center.Z - Start.Z) > P_MaxHeightDelta)
                return {};

            // 4) Overlap test against ALL channels (invalid if ANY blocks)
            const FCollisionShape Shape = FCollisionShape::MakeCapsule(Rad, Half);
            for (TEnumAsByte<ECollisionChannel> Chan : Channels)
            {
                if (W->OverlapBlockingTestByChannel(Center, FQuat::Identity, Chan, Shape, QParams))
                {
                    return {}; // blocked on this channel → invalid
                }
            }

            return Center; // valid → return the center we tested
        };

    TOptional<FVector> Furthest; // last valid is furthest
    const int32 Steps = P_NumSamples;

    for (int32 i = 1; i <= Steps; ++i)
    {
        const float T = float(i) / float(Steps);
        const FVector Candidate = Start + Dir * (P_MaxDistance * T);

        TOptional<FVector> ValidCenter = ValidateCandidate(Candidate);

#if !(UE_BUILD_SHIPPING)
        if (P_DebugSeconds > 0.f)
        {
            if (ValidCenter.IsSet())
            {
                DrawDebugCapsule(W, ValidCenter.GetValue(), Half, Rad, FQuat::Identity, FColor::Green, false, P_DebugSeconds);
            }
            else
            {
                // Draw where we sampled along the path (sphere as a hint for invalid)
                DrawDebugSphere(W, Candidate, 6.f, 10, FColor::Red, false, P_DebugSeconds);
            }
        }
#endif

        if (ValidCenter.IsSet())
        {
            Furthest = ValidCenter; // last valid is the furthest so far
        }
    }

    if (!Furthest.IsSet())
        return false;

    OutLoc = Furthest.GetValue(); // capsule center we validated
    OutRot = Dir.Rotation();
    OutTravel = FVector::Dist2D(Start, OutLoc);
    return true;
}
