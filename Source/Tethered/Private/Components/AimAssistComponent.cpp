// AimAssistComponent.cpp
#include "Components/AimAssistComponent.h"
#include "Interfaces/Aimable.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogAimAssist, Log, All);

// Global debug state - Definition of the static member variable
bool UAimAssistComponent::bGlobalDebugEnabled = false;

UAimAssistComponent::UAimAssistComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

/** Initialize component and set up target querying timer */
void UAimAssistComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Always log initialization for debugging
	UE_LOG(LogAimAssist, Warning, TEXT("=== AimAssistComponent BeginPlay ==="));
	
	OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar.IsValid())
	{
		if (AController* C = OwnerChar->GetController())
			PC = Cast<APlayerController>(C);
		UE_LOG(LogAimAssist, Warning, TEXT("Owner Character: %s"), *OwnerChar->GetName());
	}
	else
	{
		UE_LOG(LogAimAssist, Error, TEXT("NO OWNER CHARACTER!"));
	}
	
	if (Profile)
	{
		UE_LOG(LogAimAssist, Warning, TEXT("Profile: %s"), *Profile->GetName());
		UE_LOG(LogAimAssist, Warning, TEXT("  Range: %.1f"), Profile->AssistRangeCm);
		UE_LOG(LogAimAssist, Warning, TEXT("  FOV: %.1f"), Profile->QueryFOVDeg);
	}
	else
	{
		UE_LOG(LogAimAssist, Error, TEXT("NO PROFILE SET!"));
	}
	
	UE_LOG(LogAimAssist, Warning, TEXT("Query Interval: %.3f"), QueryInterval);
	UE_LOG(LogAimAssist, Warning, TEXT("ObjectTypes Count: %d"), ObjectTypes.Num());
	
	for (int32 i = 0; i < ObjectTypes.Num(); ++i)
	{
		UE_LOG(LogAimAssist, Warning, TEXT("  ObjectType[%d]: %d"), i, (int32)ObjectTypes[i]);
	}
	
	if (QueryInterval > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			QueryTimer, this, &UAimAssistComponent::QueryForTarget,
			QueryInterval, true, 0.05f);
		UE_LOG(LogAimAssist, Warning, TEXT("Timer started successfully"));
	}
	else
	{
		UE_LOG(LogAimAssist, Error, TEXT("Query interval is 0 or negative! Timer not started!"));
	}
	
	UE_LOG(LogAimAssist, Warning, TEXT("=== BeginPlay Complete ==="));
}

/** Clean up timer when component is destroyed */
void UAimAssistComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(QueryTimer);
	Super::EndPlay(EndPlayReason);
}

/** Main tick function - applies aim assist if target exists */
void UAimAssistComponent::TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*)
{
	if (!Profile || !OwnerChar.IsValid()) return;
	
	if (CurrentTarget.IsValid())
	{
		ApplyAssist(Dt);
	}
	
	// Draw debug information if enabled
	if (bGlobalDebugEnabled)
	{
		DrawDebugInfo();
	}
}

/** Gets the player character's world position */
FVector UAimAssistComponent::GetPlayerPos() const
{
	return OwnerChar.IsValid() ? OwnerChar->GetActorLocation() : FVector::ZeroVector;
}

/** Gets the character's forward direction in 2D from yaw rotation (used for all targeting) */
FVector2D UAimAssistComponent::GetTargetingDirection2D() const
{
	if (!OwnerChar.IsValid()) return FVector2D(1, 0);
	const FRotator YawOnly(0.f, OwnerChar->GetActorRotation().Yaw, 0.f);
	const FVector Fwd = YawOnly.Vector();
	return FVector2D(Fwd.X, Fwd.Y).GetSafeNormal();
}

/** Calculates angle between two 2D vectors in degrees */
float UAimAssistComponent::Angle2D(const FVector2D& A, const FVector2D& B)
{
	const float dot = FMath::Clamp(FVector2D::DotProduct(A.GetSafeNormal(), B.GetSafeNormal()), -1.f, 1.f);
	return FMath::RadiansToDegrees(FMath::Acos(dot));
}

/** Gets the designated aim point component from an Aimable target */
USceneComponent* UAimAssistComponent::GetAimPointComp(AActor* Target) const
{
	if (!Target) return nullptr;
	
	// Try to use the interface directly first (more efficient)
	if (IAimable* AimableTarget = Cast<IAimable>(Target))
	{
		// Use the interface method directly
		USceneComponent* AimPoint = AimableTarget->Execute_GetAimPointComponent(Target);
		if (AimPoint) return AimPoint;
	}
	
	// Fallback to reflection method if direct interface call fails
	if (Target->GetClass()->ImplementsInterface(UAimable::StaticClass()))
	{
		if (UFunction* Fn = Target->FindFunction(TEXT("GetAimPointComponent")))
		{
			USceneComponent* Out = nullptr;
			Target->ProcessEvent(Fn, &Out);
			if (Out) return Out;
		}
	}
	
	// Final fallback to root component
	return Target->GetRootComponent();
}

/** Gets the world position of the target's aim point */
FVector UAimAssistComponent::GetAimPointWorld(AActor* Target) const
{
	if (USceneComponent* C = GetAimPointComp(Target)) return C->GetComponentLocation();
	return Target ? Target->GetActorLocation() : FVector::ZeroVector;
}

/** Checks if target direction is within the FOV cone */
bool UAimAssistComponent::PassesFOV2D(const FVector& ToTarget2D) const
{
	if (!Profile) return false;
	const FVector2D CharacterForward = GetTargetingDirection2D();
	const FVector2D To = FVector2D(ToTarget2D.X, ToTarget2D.Y).GetSafeNormal();
	const float dot = FVector2D::DotProduct(CharacterForward, To);
	const float minCos = FMath::Cos(FMath::DegreesToRadians(Profile->QueryFOVDeg));
	return dot >= minCos;
}

/** Performs line trace to check clear line of sight to target */
bool UAimAssistComponent::HasLineOfSightToAimPoint(AActor* Target) const
{
	if (!OwnerChar.IsValid() || !Target) return false;
	const FVector From = GetPlayerPos() + FVector(0, 0, 50);
	const FVector To = GetAimPointWorld(Target);
	FHitResult Hit;
	FCollisionQueryParams P(SCENE_QUERY_STAT(AimAssist_LOS), false, GetOwner());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, From, To, LOSChannel, P);
	return (!bHit) || (Hit.GetActor() == Target);
}

/** Calculates priority score for target selection based on multiple factors */
float UAimAssistComponent::ScoreTarget(AActor* Target, float Dist2D, float Dot2D, bool bIsCurrent) const
{
	FVector Vel = FVector::ZeroVector;
	if (const UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
		Vel = Prim->GetComponentVelocity();
	const FVector2D V2(Vel.X, Vel.Y);
	const FVector2D CharacterForward = GetTargetingDirection2D();
	const float VelAlign = V2.IsNearlyZero() ? 0.f : FVector2D::DotProduct(V2.GetSafeNormal(), CharacterForward);

	const float distTerm = (Dist2D > KINDA_SMALL_NUMBER) ? (1.f / Dist2D) : 1.f;
	const float sticky = bIsCurrent ? 1.f : 0.f;

	return WDist * distTerm + WAngle * Dot2D + WVel * VelAlign + WSticky * sticky;
}

/** Searches for and selects the best target within range */
void UAimAssistComponent::QueryForTarget()
{
	// Always log entry for debugging
	UE_LOG(LogAimAssist, Warning, TEXT("=== QueryForTarget Called ==="));
	
	if (!Profile)
	{
		UE_LOG(LogAimAssist, Error, TEXT("QueryForTarget: NO PROFILE! Exiting early."));
		return;
	}
	
	if (!OwnerChar.IsValid())
	{
		UE_LOG(LogAimAssist, Error, TEXT("QueryForTarget: NO OWNER CHARACTER! Exiting early."));
		return;
	}

	const FVector Center = GetPlayerPos();
	TArray<AActor*> Ignore; Ignore.Add(GetOwner());
	TArray<AActor*> Hits;

	// Always log search parameters
	UE_LOG(LogAimAssist, Warning, TEXT("Search Parameters:"));
	UE_LOG(LogAimAssist, Warning, TEXT("  Center: %s"), *Center.ToString());
	UE_LOG(LogAimAssist, Warning, TEXT("  Range: %.1f"), Profile->AssistRangeCm);
	UE_LOG(LogAimAssist, Warning, TEXT("  ObjectTypes Count: %d"), ObjectTypes.Num());

	// TEMPORARY: Try searching with AActor::StaticClass() to bypass collision issues
	UKismetSystemLibrary::SphereOverlapActors(
		this, Center, Profile->AssistRangeCm,
		ObjectTypes, AActor::StaticClass(), Ignore, Hits);

	// Always log results
	UE_LOG(LogAimAssist, Warning, TEXT("SphereOverlapActors Result: Found %d actors"), Hits.Num());
	
	if (Hits.Num() == 0)
	{
		UE_LOG(LogAimAssist, Error, TEXT("NO ACTORS FOUND! Possible issues:"));
		UE_LOG(LogAimAssist, Error, TEXT("1. No actors in range"));
		UE_LOG(LogAimAssist, Error, TEXT("2. ObjectTypes mismatch (current count: %d)"), ObjectTypes.Num());
		UE_LOG(LogAimAssist, Error, TEXT("3. Range too small: %.1f"), Profile->AssistRangeCm);
		UE_LOG(LogAimAssist, Error, TEXT("4. Actors don't have proper collision setup"));
		CurrentTarget = nullptr;
		return;
	}

	// Log all found actors
	for (int32 i = 0; i < Hits.Num(); ++i)
	{
		AActor* Actor = Hits[i];
		if (Actor)
		{
			bool bImplementsAimable = Actor->GetClass()->ImplementsInterface(UAimable::StaticClass());
			UE_LOG(LogAimAssist, Warning, TEXT("  [%d] Actor: %s (Class: %s) - Aimable: %s"), 
				i, *Actor->GetName(), *Actor->GetClass()->GetName(),
				bImplementsAimable ? TEXT("YES") : TEXT("NO"));
		}
		else
		{
			UE_LOG(LogAimAssist, Warning, TEXT("  [%d] NULL ACTOR"), i);
		}
	}

	AActor* Best = nullptr;
	float BestScore = -FLT_MAX;

	// Clear debug arrays
	DebugPotentialTargets.Empty();
	DebugTargetScores.Empty();

	// Score the current target too, for hysteresis comparison
	float CurrentScore = -FLT_MAX;
	AActor* Curr = CurrentTarget.Get();
	if (Curr)
	{
		UE_LOG(LogAimAssist, Warning, TEXT("Current target exists: %s"), *Curr->GetName());
		const FVector To = GetAimPointWorld(Curr) - Center;
		const FVector To2D = FVector(To.X, To.Y, 0.f);
		const float Dist2D = To2D.Size();
		const FVector2D CharacterForward = GetTargetingDirection2D();
		const float Dot2D = FVector2D::DotProduct(CharacterForward, FVector2D(To2D.X, To2D.Y).GetSafeNormal());
		if (PassesFOV2D(To2D) && HasLineOfSightToAimPoint(Curr))
			CurrentScore = ScoreTarget(Curr, Dist2D, Dot2D, true);
	}
	else
	{
		UE_LOG(LogAimAssist, Warning, TEXT("No current target"));
	}

	int32 ValidTargets = 0;
	int32 TargetsInFOV = 0;
	int32 TargetsWithLOS = 0;
	int32 ProcessedActors = 0;

	// Evaluate all potential targets
	for (AActor* A : Hits)
	{
		ProcessedActors++;
		UE_LOG(LogAimAssist, Warning, TEXT("Processing actor %d: %s"), ProcessedActors, A ? *A->GetName() : TEXT("NULL"));
		
		if (!A || A == GetOwner()) 
		{
			UE_LOG(LogAimAssist, Warning, TEXT("  Skipping: NULL or owner"));
			continue;
		}
		
		// Check if it implements IAimable interface
		bool bImplementsInterface = A->GetClass()->ImplementsInterface(UAimable::StaticClass());
		UE_LOG(LogAimAssist, Warning, TEXT("  Implements IAimable: %s"), bImplementsInterface ? TEXT("YES") : TEXT("NO"));
		
		if (!bImplementsInterface) 
		{
			UE_LOG(LogAimAssist, Warning, TEXT("  Skipping: Doesn't implement IAimable"));
			continue;
		}

		// Check if target can be targeted using the IAimable interface
		bool bCanBeTargeted = true;
		if (IAimable* AimableTarget = Cast<IAimable>(A))
		{
			bCanBeTargeted = AimableTarget->Execute_CanBeTargeted(A);
			UE_LOG(LogAimAssist, Warning, TEXT("  Direct interface call - CanBeTargeted: %s"), bCanBeTargeted ? TEXT("YES") : TEXT("NO"));
		}
		else
		{
			// Fallback to reflection method if direct cast fails
			if (UFunction* Fn = A->FindFunction(TEXT("CanBeTargeted")))
			{
				A->ProcessEvent(Fn, &bCanBeTargeted);
				UE_LOG(LogAimAssist, Warning, TEXT("  Reflection call - CanBeTargeted: %s"), bCanBeTargeted ? TEXT("YES") : TEXT("NO"));
			}
			else
			{
				UE_LOG(LogAimAssist, Warning, TEXT("  No CanBeTargeted function found, assuming true"));
			}
		}
		
		if (!bCanBeTargeted) 
		{
			UE_LOG(LogAimAssist, Warning, TEXT("  Skipping: CanBeTargeted returned false"));
			continue;
		}
		ValidTargets++;

		const FVector To = GetAimPointWorld(A) - Center;
		const FVector To2D = FVector(To.X, To.Y, 0.f);
		const float Dist2D = To2D.Size();
		
		UE_LOG(LogAimAssist, Warning, TEXT("  Distance: %.1f"), Dist2D);
		
		if (Dist2D <= KINDA_SMALL_NUMBER) 
		{
			UE_LOG(LogAimAssist, Warning, TEXT("  Skipping: Too close (distance <= %f)"), KINDA_SMALL_NUMBER);
			continue;
		}

		const FVector2D CharacterForward = GetTargetingDirection2D();
		const float Dot2D = FVector2D::DotProduct(CharacterForward, FVector2D(To2D.X, To2D.Y).GetSafeNormal());
		const float S = ScoreTarget(A, Dist2D, Dot2D, A == Curr);

		// Store for debug visualization
		DebugPotentialTargets.Add(A);
		DebugTargetScores.Add(S);

		// Check FOV and LOS
		bool bInFOV = PassesFOV2D(To2D);
		bool bHasLOS = HasLineOfSightToAimPoint(A);
		
		if (bInFOV) TargetsInFOV++;
		if (bHasLOS) TargetsWithLOS++;

		UE_LOG(LogAimAssist, Warning, TEXT("  InFOV: %s, HasLOS: %s, Score: %.2f"), 
			bInFOV ? TEXT("YES") : TEXT("NO"), bHasLOS ? TEXT("YES") : TEXT("NO"), S);

		// Only consider targets that pass FOV and LOS checks for actual targeting
		if (bInFOV && bHasLOS)
		{
			if (S > BestScore) 
			{ 
				BestScore = S; 
				Best = A; 
				UE_LOG(LogAimAssist, Warning, TEXT("  NEW BEST TARGET: %s (Score: %.2f)"), *A->GetName(), S);
			}
		}
		else
		{
			UE_LOG(LogAimAssist, Warning, TEXT("  Failed FOV or LOS check"));
		}
	}

	// Always log the final summary
	UE_LOG(LogAimAssist, Warning, TEXT("=== FINAL RESULTS ==="));
	UE_LOG(LogAimAssist, Warning, TEXT("Processed: %d, Valid: %d, InFOV: %d, WithLOS: %d"), 
		ProcessedActors, ValidTargets, TargetsInFOV, TargetsWithLOS);
	UE_LOG(LogAimAssist, Warning, TEXT("Selected Target: %s"), Best ? *Best->GetName() : TEXT("NONE"));

	// Hysteresis: require new target to beat current by a margin (prevents jittering)
	if (Best && Curr && Best != Curr)
	{
		const float margin = FMath::Lerp(0.f, 0.25f, Profile->Stickiness); // up to +25% better required
		if (BestScore < CurrentScore * (1.f + margin))
		{
			Best = Curr; // keep current
			UE_LOG(LogAimAssist, Warning, TEXT("Hysteresis: Keeping current target due to margin"));
		}
	}
	
	CurrentTarget = Best;
	UE_LOG(LogAimAssist, Warning, TEXT("=== QueryForTarget Complete ==="));
}

/** Returns assist strength based on input magnitude - stronger with lighter input */
float UAimAssistComponent::AssistStrengthByStick() const
{
	const float t = FMath::Clamp(AimInputMagnitude, 0.f, 1.f);
	return FMath::Lerp(1.f, 0.25f, t); // strong assist at low input, weak at high
}

/** Rotates character toward target by specified degrees with proper direction */
void UAimAssistComponent::ApplyYawToward(const FVector2D& ToTarget2D, float Degrees)
{
	if (!OwnerChar.IsValid()) return;
	const FVector2D CharacterForward = GetTargetingDirection2D();
	const float crossZ = CharacterForward.X * ToTarget2D.Y - CharacterForward.Y * ToTarget2D.X;
	const float sign = (crossZ >= 0.f) ? 1.f : -1.f;

	FRotator R = OwnerChar->GetActorRotation();
	R.Yaw += Degrees * sign;
	R.Pitch = 0.f; // keep iso pitch stable
	OwnerChar->SetActorRotation(R);
}

/** Applies various forms of aim assistance based on profile settings */
void UAimAssistComponent::ApplyAssist(float Dt)
{
	if (!Profile || !CurrentTarget.IsValid()) return;

	const FVector P = GetPlayerPos();
	FVector To = GetAimPointWorld(CurrentTarget.Get()) - P; To.Z = 0.f;
	FVector2D To2D(To.X, To.Y); To2D.Normalize();

	const FVector2D CharacterForward = GetTargetingDirection2D();
	const float Angle = Angle2D(CharacterForward, To2D);

	// Note: Magnetism (bending aim vector) is removed since we use character forward direction only
	// Friction should be implemented in camera/look code by reading the current target state

	// (1) Soft snap (controller/touch only—gate by your input mode)
	const bool bAllowSnap = true;
	if (bAllowSnap && Angle <= Profile->SnapConeDeg)
	{
		ApplyYawToward(To2D, FMath::Min(Angle, Profile->MaxSnapDeg));
	}

	// (2) Rotational assist - turn character toward target
	const float MaxStep = Profile->MaxYawDegPerSec * Dt * AssistStrengthByStick();
	if (MaxStep > 0.f && Angle > 0.01f)
	{
		ApplyYawToward(To2D, FMath::Min(Angle, MaxStep));
	}
}

/** Applies turn assist and lunge movement for melee attacks */
void UAimAssistComponent::OnMeleeCommit()
{
	if (!Profile || !CurrentTarget.IsValid() || !OwnerChar.IsValid()) return;

	const FVector P = GetPlayerPos();
	FVector To = GetAimPointWorld(CurrentTarget.Get()) - P; To.Z = 0.f;
	FVector2D To2D(To.X, To.Y); To2D.Normalize();

	const FVector2D CharacterForward = GetTargetingDirection2D();
	const float Angle = Angle2D(CharacterForward, To2D);
	const float Turn = FMath::Min(Profile->MeleeMaxTurnOnAttack, Angle);
	ApplyYawToward(To2D, Turn);

	const float Dist = FMath::Min(Profile->MeleeLungeDistCm, FVector::Dist2D(P, GetAimPointWorld(CurrentTarget.Get())));
	const FVector LungeVel = FVector(To2D.X, To2D.Y, 0.f) * Profile->MeleeLungeSpeed;
	OwnerChar->LaunchCharacter(LungeVel * 0.1f, true, false);
}

/** Applies steering and optional homing to fired projectiles */
void UAimAssistComponent::OnRangedFire(AActor* Projectile, UProjectileMovementComponent* Move)
{
	if (!Profile || !CurrentTarget.IsValid() || !Projectile) return;

	const FVector From = Projectile->GetActorForwardVector();
	const FVector To3 = GetAimPointWorld(CurrentTarget.Get()) - Projectile->GetActorLocation();
	FVector2D From2D(From.X, From.Y); From2D.Normalize();
	FVector2D To2D(To3.X, To3.Y);   To2D.Normalize();

	const float Angle = Angle2D(From2D, To2D);
	const float Step = FMath::Min(Angle, Profile->ProjectileSteerDeg);

	const float crossZ = From2D.X * To2D.Y - From2D.Y * To2D.X;
	const float sign = (crossZ >= 0.f) ? 1.f : -1.f;
	FRotator R = Projectile->GetActorRotation();
	R.Yaw += Step * sign;
	Projectile->SetActorRotation(R);

	// Set up temporary homing if enabled
	if (Move && Profile->InitialHomingTime > 0.f)
	{
		Move->bIsHomingProjectile = true;
		Move->HomingAccelerationMagnitude = FMath::Max(Move->HomingAccelerationMagnitude, 8000.f);
		if (USceneComponent* HomingComp = GetAimPointComp(CurrentTarget.Get()))
			Move->HomingTargetComponent = HomingComp;

		FTimerHandle DisableHoming;
		GetWorld()->GetTimerManager().SetTimer(DisableHoming, FTimerDelegate::CreateWeakLambda(Move, [Move]()
			{
				if (Move) Move->bIsHomingProjectile = false;
			}), Profile->InitialHomingTime, false);
	}
}

#pragma region Debug Visualization

/** Draws debug information when global debug is enabled */
void UAimAssistComponent::DrawDebugInfo()
{
	if (!Profile || !OwnerChar.IsValid() || !GetWorld()) return;

	DrawDetectionRange();
	DrawFOVCone();
	DrawAssistCones();
	DrawLineOfSightTraces();
	DrawTargetInfo();
}

/** Draws the detection range sphere */
void UAimAssistComponent::DrawDetectionRange()
{
	const FVector Center = GetPlayerPos();
	const float Radius = Profile->AssistRangeCm;
	
	// Draw detection sphere (wireframe)
	DrawDebugSphere(GetWorld(), Center, Radius, 128, FColor::Blue, false, -1.f, 0, 0.5f);
	
	// Draw range text
	DrawDebugString(GetWorld(), Center + FVector(0, 0, Radius + 50), 
		FString::Printf(TEXT("Range: %.0fcm"), Radius), nullptr, FColor::Blue, 0.f);
}

/** Draws the FOV cone */
void UAimAssistComponent::DrawFOVCone()
{
	const FVector Center = GetPlayerPos();
	const FVector2D CharacterForward = GetTargetingDirection2D();
	const FVector CharacterForward3D = FVector(CharacterForward.X, CharacterForward.Y, 0).GetSafeNormal();
	
	const float FOVAngle = Profile->QueryFOVDeg;
	const float ConeLength = Profile->AssistRangeCm * 0.8f; // Slightly shorter than detection range
	
	// Draw FOV cone edges
	const float HalfAngleRad = FMath::DegreesToRadians(FOVAngle * 0.5f);
	
	// Calculate cone edge directions
	const FVector RightEdge = CharacterForward3D.RotateAngleAxis(FOVAngle * 0.5f, FVector::UpVector);
	const FVector LeftEdge = CharacterForward3D.RotateAngleAxis(-FOVAngle * 0.5f, FVector::UpVector);
	
	// Draw cone lines
	DrawDebugLine(GetWorld(), Center, Center + RightEdge * ConeLength, FColor::Green, false, -1.f, 0, 1.f);
	DrawDebugLine(GetWorld(), Center, Center + LeftEdge * ConeLength, FColor::Green, false, -1.f, 0, 1.f);
	DrawDebugLine(GetWorld(), Center, Center + CharacterForward3D * ConeLength, FColor::Green, false, -1.f, 0, 2.f);
	
	// Draw FOV arc
	const int32 NumSegments = 16;
	for (int32 i = 0; i < NumSegments; ++i)
	{
		const float Angle1 = -FOVAngle * 0.5f + (FOVAngle * i / NumSegments);
		const float Angle2 = -FOVAngle * 0.5f + (FOVAngle * (i + 1) / NumSegments);
		
		const FVector Dir1 = CharacterForward3D.RotateAngleAxis(Angle1, FVector::UpVector);
		const FVector Dir2 = CharacterForward3D.RotateAngleAxis(Angle2, FVector::UpVector);
		
		DrawDebugLine(GetWorld(), Center + Dir1 * ConeLength, Center + Dir2 * ConeLength, FColor::Green, false, -1.f, 0, 1.f);
	}
	
	// Draw FOV text
	DrawDebugString(GetWorld(), Center + CharacterForward3D * (ConeLength + 100), 
		FString::Printf(TEXT("FOV: %.1f° (Character Forward)"), FOVAngle), nullptr, FColor::Green, 0.f);
}

/** Draws assist cones (friction, snap) */
void UAimAssistComponent::DrawAssistCones()
{
	if (!CurrentTarget.IsValid()) return;
	
	const FVector Center = GetPlayerPos();
	const FVector TargetPos = GetAimPointWorld(CurrentTarget.Get());
	const FVector ToTarget = (TargetPos - Center).GetSafeNormal();
	const FVector2D CharacterForward = GetTargetingDirection2D();
	const FVector CharacterForward3D = FVector(CharacterForward.X, CharacterForward.Y, 0).GetSafeNormal();
	
	const float ConeLength = FVector::Dist(Center, TargetPos) * 0.7f;
	
	// Draw friction cone (Orange) - for camera/look system reference
	const float FrictionAngle = Profile->FrictionConeDeg;
	DrawDebugCone(GetWorld(), Center, CharacterForward3D, ConeLength * 0.8f, 
		FMath::DegreesToRadians(FrictionAngle * 0.5f), 
		FMath::DegreesToRadians(FrictionAngle * 0.5f), 8, FColor::Orange, false, -1.f, 0, 1.f);
	
	// Draw snap cone (Red)
	const float SnapAngle = Profile->SnapConeDeg;
	DrawDebugCone(GetWorld(), Center, CharacterForward3D, ConeLength * 0.6f, 
		FMath::DegreesToRadians(SnapAngle * 0.5f), 
		FMath::DegreesToRadians(SnapAngle * 0.5f), 6, FColor::Red, false, -1.f, 0, 2.f);
	
	// Draw assist info (removed magnetism since it's not used)
	const FVector InfoPos = Center + FVector(0, 0, 100);
	DrawDebugString(GetWorld(), InfoPos, 
		FString::Printf(TEXT("Friction: %.1f° | Snap: %.1f°"), 
			FrictionAngle, SnapAngle), nullptr, FColor::Yellow, 0.f);
	
	// Show current angle to target
	const FVector2D ToTarget2D = FVector2D(ToTarget.X, ToTarget.Y).GetSafeNormal();
	const float AngleToTarget = Angle2D(CharacterForward, ToTarget2D);
	DrawDebugString(GetWorld(), InfoPos + FVector(0, 0, 30), 
		FString::Printf(TEXT("Angle to Target: %.1f° (Char Forward)"), AngleToTarget), nullptr, FColor::White, 0.f);
}

/** Draws line of sight traces to targets */
void UAimAssistComponent::DrawLineOfSightTraces()
{
	const FVector Center = GetPlayerPos() + FVector(0, 0, 50);
	
	// Draw LOS to current target
	if (CurrentTarget.IsValid())
	{
		const FVector TargetPos = GetAimPointWorld(CurrentTarget.Get());
		const bool bHasLOS = HasLineOfSightToAimPoint(CurrentTarget.Get());
		const FColor LOSColor = bHasLOS ? FColor::Green : FColor::Red;
		
		DrawDebugLine(GetWorld(), Center, TargetPos, LOSColor, false, -1.f, 0, 3.f);
		DrawDebugSphere(GetWorld(), TargetPos, 25.f, 8, LOSColor, false, -1.f, 0, 2.f);
	}
	
	// Draw LOS to other potential targets
	for (AActor* Target : DebugPotentialTargets)
	{
		if (Target && Target != CurrentTarget.Get())
		{
			const FVector TargetPos = GetAimPointWorld(Target);
			const bool bHasLOS = HasLineOfSightToAimPoint(Target);
			const FColor LOSColor = bHasLOS ? FColor::Green : FColor::Red;
			
			DrawDebugLine(GetWorld(), Center, TargetPos, LOSColor, false, -1.f, 0, 1.f);
			DrawDebugSphere(GetWorld(), TargetPos, 15.f, 6, LOSColor, false, -1.f, 0, 1.f);
		}
	}
}

/** Draws target information and scores */
void UAimAssistComponent::DrawTargetInfo()
{
	// Draw current target info
	if (CurrentTarget.IsValid())
	{
		const FVector TargetPos = GetAimPointWorld(CurrentTarget.Get());
		const FString TargetName = CurrentTarget->GetName();
		
		DrawDebugString(GetWorld(), TargetPos + FVector(0, 0, 100), 
			FString::Printf(TEXT("CURRENT TARGET: %s"), *TargetName), nullptr, FColor::Green, 0.f, true);
	}
	
	// Draw scores for all potential targets
	for (int32 i = 0; i < DebugPotentialTargets.Num(); ++i)
	{
		AActor* Target = DebugPotentialTargets[i];
		if (!Target) continue;
		
		const float Score = DebugTargetScores[i];
		const FVector TargetPos = GetAimPointWorld(Target);
		const bool bIsCurrentTarget = Target == CurrentTarget.Get();
		const FColor ScoreColor = bIsCurrentTarget ? FColor::Green : FColor::Yellow;
		
		DrawDebugString(GetWorld(), TargetPos + FVector(0, 0, 50), 
			FString::Printf(TEXT("Score: %.2f"), Score), nullptr, ScoreColor, 0.f, true);
		
		// Draw target name
		DrawDebugString(GetWorld(), TargetPos + FVector(0, 0, 75), 
			Target->GetName(), nullptr, ScoreColor, 0.f, true);
	}
	
	// Draw input magnitude and assist strength
	const FVector PlayerPos = GetPlayerPos();
	DrawDebugString(GetWorld(), PlayerPos + FVector(0, 0, 150), 
		FString::Printf(TEXT("Input Mag: %.2f | Assist Strength: %.2f"), 
			AimInputMagnitude, AssistStrengthByStick()), nullptr, FColor::Cyan, 0.f, true);
}
#pragma endregion Debug Visualization
