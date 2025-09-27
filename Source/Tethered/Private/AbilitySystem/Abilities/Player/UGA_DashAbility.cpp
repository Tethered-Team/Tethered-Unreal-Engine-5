#include "AbilitySystem/Abilities/Player/UGA_DashAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Abilities/Tasks/AbilityTask_DashQuery.h"
#include "Blueprint/MotionWarpingBPLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TetheredCollisionChannels.h"

// ============================================================================
// Constructor
// ============================================================================

UGA_DashAbility::UGA_DashAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

// ============================================================================
// Gameplay Ability System Overrides
// ============================================================================

void UGA_DashAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!CachedCharacter.IsValid() || !Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Reset per-activation state so subsequent dashes can end properly
	bEndedAbilityEarly = false;
	bDashAttackWindowOpen = false;
	MontageTask.Reset();

	// 1) Query the endpoint synchronously (task fires immediately)
	const FVector DesiredDir = ComputeDesiredDirection();

	CachedCharacter->SetActorRotation(DesiredDir.Rotation());

	TArray<TEnumAsByte<ECollisionChannel>> Channels = CollisionChannels;
	if (Channels.Num() == 0)
	{
		Channels.Add(ECC_Pawn);
		// Add DashGhost channel to default collision checking to ensure proper dash collision detection
		Channels.Add(TetheredCollisionChannels::DashGhost);
	}

	auto* Query = UAbilityTask_DashQuery::DashSampleEndpoint_MultiChannel(
		this, MaxDistance, NumSamples, DesiredDir, bProjectToNav, bSnapToGround,
		MaxHeightDelta, ClearanceBuffer, Channels, DebugSeconds);
	Query->OnResult.AddDynamic(this, &UGA_DashAbility::OnDashQueryResult);
	Query->ReadyForActivation();

	if (bUseDashGhostCollision)
	{
		EnableDashGhostCollision();
	}
}

void UGA_DashAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
			FString::Printf(TEXT("%s: EndAbility Called: (cancelled=%d)"), *GetName(), bWasCancelled ? 1 : 0));
	}
#endif

	if (bUseDashGhostCollision)
	{
		DisableDashGhostCollision();
	}

	// Variant hook: end signal
	OnDashEnded(bWasCancelled);

	// Clean up pointer so next activation is fresh
	MontageTask.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ============================================================================
// Blueprint Native Event Implementations
// ============================================================================

FVector UGA_DashAbility::ComputeDesiredDirection_Implementation() const
{
	switch (DirectionSource)
	{
		case EDashDirectionSource::MovementInput:
		{
			if (CachedCharacter.IsValid())
			{
				const FVector Input = CachedCharacter->GetCharacterMovement()->GetLastInputVector();
				if (!Input.IsNearlyZero())
				{
					return Input.GetSafeNormal();
				}
			}
			// fallback to forward
		}
		[[fallthrough]];
		case EDashDirectionSource::AutoAim:
		{
			if (CachedCharacter.IsValid())
			{
				FVector Direction = FVector::ZeroVector;
				if (GetAutoAimDirection(Direction))
				{
					return Direction;
				}
			}
			// fallback to forward
		}
		[[fallthrough]];
		case EDashDirectionSource::Custom:
		{
			if (CachedCharacter.IsValid())
			{
				FVector Direction = FVector::ZeroVector;
				if (GetCustomDirection(Direction))
				{
					return Direction;
				}
			}
			// fallback to forward
		}
		[[fallthrough]];
		default:
		{
			return DefaultDirectionComputation();
		}
	}
}


// ============================================================================
// Internal Event Handlers
// ============================================================================

void UGA_DashAbility::OnDashQueryResult(bool bFound, FVector EndLocation, FRotator Facing, float Travel)
{
	if (!bFound || !CachedCharacter.IsValid() || !Montage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	ChosenEnd = EndLocation;
	ChosenFacing = Facing;

	// Variant hook: dash is about to start
	DashStartLoc = CachedCharacter->GetActorLocation();
	OnDashStarted(DashStartLoc, ChosenEnd);

	OnDashTargetChosen(EndLocation, Facing, Travel); // C++ virtual extension point

	ReadyDashMontage(Facing, EndLocation);

	// 3) Listen for handoff GameplayEvent (from an AnimNotify in the montage)
	SetupHandoffEvents();


	BindAttackWindowEvents();
}

void UGA_DashAbility::BindAttackWindowEvents()
{
	// 4) Optional: variant-driven attack window & input — only if variant set tags
	if (AttackWindowOpenTag.IsValid())
	{
		auto* WOpen = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AttackWindowOpenTag, nullptr, false, false);
		WOpen->EventReceived.AddDynamic(this, &UGA_DashAbility::OnAttackWindowOpen);
		WOpen->ReadyForActivation();
	}
	if (AttackWindowCloseTag.IsValid())
	{
		auto* WClose = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AttackWindowCloseTag, nullptr, false, false);
		WClose->EventReceived.AddDynamic(this, &UGA_DashAbility::OnAttackWindowClose);
		WClose->ReadyForActivation();
	}
	if (DashAttackInputTag.IsValid())
	{
		auto* WInput = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DashAttackInputTag, nullptr, false, false);
		WInput->EventReceived.AddDynamic(this, &UGA_DashAbility::OnAttackInputEvent);
		WInput->ReadyForActivation();
	}
}

void UGA_DashAbility::OnHandoffEvent(FGameplayEventData Payload)
{
	// Variant hook: exact handoff moment
	OnDashHandoff();


	bEndedAbilityEarly = true; // mark so Completed handler doesn't double-end
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*rep*/true, /*cancelled*/false);
}

void UGA_DashAbility::OnMontageCompleted()
{
	// Montage finished; if we already ended early, nothing to do.
	if (!bEndedAbilityEarly)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_DashAbility::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_DashAbility::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_DashAbility::OnMontageBlendOut()
{

	// Then end so input comes back ASAP
	if (!bEndedAbilityEarly)
	{
		bEndedAbilityEarly = true;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*rep*/true, /*cancelled*/false);
	}
}

// Attack Window Event Handlers
void UGA_DashAbility::OnAttackWindowOpen(FGameplayEventData /*Payload*/)
{
	bDashAttackWindowOpen = true;
	OnDashAttackWindowOpened();
}

void UGA_DashAbility::OnAttackWindowClose(FGameplayEventData /*Payload*/)
{
	bDashAttackWindowOpen = false;
	OnDashAttackWindowClosed();
}

void UGA_DashAbility::OnAttackInputEvent(FGameplayEventData Payload)
{
	if (!bDashAttackWindowOpen) return;

	// Let BP or C++ child decide what to do (e.g., end dash, start dash-attack GA). Return true if handled.
	if (OnDashAttackInput(Payload))
	{
		return;
	}
	// Base does nothing by default.
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

void UGA_DashAbility::SetupFallbackEarlyEndTimer()
{
	if (!CachedCharacter.IsValid() || !Montage) return;
	
	// Fire a timer roughly at the requested normalized time in the current section
	const int32 SecIdx = Montage->GetSectionIndex(MontageSection);
	if (SecIdx == INDEX_NONE) return;
	
	const float SectionLen = Montage->GetSectionLength(SecIdx);
	if (SectionLen <= 0.f) return;
	
	const float Delay = FMath::Clamp(FallbackEarlyEndAtSectionTimeRatio, 0.f, 1.f) * SectionLen;
	FTimerHandle H;
	CachedCharacter->GetWorldTimerManager().SetTimer(H, [this]()
		{
			if (!bEndedAbilityEarly) 
			{ 
				OnHandoffEvent(FGameplayEventData()); 
			}
		}, Delay, false);
}

FVector UGA_DashAbility::DefaultDirectionComputation() const
{
	if (const ACharacter* C = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		return C->GetActorForwardVector().GetSafeNormal2D();
	}
	return FVector::ForwardVector;
}

void UGA_DashAbility::EnableDashGhostCollision()
{
	ACharacter* Char = CachedCharacter.Get();
	if (!Char || bDashGhostActive) return;

	UCapsuleComponent* Cap = Char->GetCapsuleComponent();
	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	if (!Cap || !CMC) return;

	// Save current state
	SavedCapsuleProfile = Cap->GetCollisionProfileName();
	SavedMovementMode = CMC->MovementMode;
	SavedGravityScale = CMC->GravityScale;

	CMC->GravityScale = DashGravityScale;
	CMC->MovementMode = DashMovementMode;
	Cap->SetCollisionObjectType(DashGhostCollisionObjectType);

	bDashGhostActive = true;
}

void UGA_DashAbility::DisableDashGhostCollision()
{
	ACharacter* Char = CachedCharacter.Get();
	if (!Char || !bDashGhostActive) return;

	UCapsuleComponent* Cap = Char->GetCapsuleComponent();
	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	if (!Cap || !CMC) 
	{ 
		bDashGhostActive = false; 
		return; 
	}

	// Restore movement first, then collision profile/flags
	CMC->SetMovementMode(SavedMovementMode);
	CMC->GravityScale = SavedGravityScale;
	Cap->SetCollisionProfileName(SavedCapsuleProfile);

	bDashGhostActive = false;
}

void UGA_DashAbility::ReadyDashMontage(FRotator Facing, FVector EndLocation)
{
	// Allow variant to swap montage/section
	UAnimMontage* MontageToPlay = Montage;
	FName SectionToPlay = MontageSection;
	//OnBeforePlayMontage(MontageToPlay, SectionToPlay);

	// 2) Feed Motion Warping + play montage with StopWhenAbilityEnds=false (EARLY END allowed)
	UMotionWarpingBPLibrary::AddOrUpdateWarpTarget(CachedCharacter.Get(), WarpSyncPoint, FTransform(Facing, EndLocation));

	auto* Play = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay, /*Rate*/1.f, /*StartSection*/SectionToPlay,
		/*bStopWhenAbilityEnds*/ false, /*RootMotionScale*/1.f, /*StartTime*/0.f,
		/*bAllowInterruptAfterBlendOut*/ false);

	MontageTask = Play;
	Play->OnCompleted.AddDynamic(this, &UGA_DashAbility::OnMontageCompleted);
	Play->OnInterrupted.AddDynamic(this, &UGA_DashAbility::OnMontageInterrupted);
	Play->OnCancelled.AddDynamic(this, &UGA_DashAbility::OnMontageCancelled);
	Play->OnBlendOut.AddDynamic(this, &UGA_DashAbility::OnMontageBlendOut);
	Play->ReadyForActivation();
}


void UGA_DashAbility::SetupHandoffEvents()
{
	if (HandoffEventTag.IsValid())
	{
		auto* WaitEvt = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HandoffEventTag, nullptr, /*OnlyTriggerOnce*/false, /*OnlyOnServer*/false);
		WaitEvt->EventReceived.AddDynamic(this, &UGA_DashAbility::OnHandoffEvent);
		WaitEvt->ReadyForActivation();
	}
	else if (FallbackEarlyEndAtSectionTimeRatio > 0.f)
	{
		SetupFallbackEarlyEndTimer();
	}
}

//=================================================================================
// Blueprint Native Events
//=================================================================================

	void UGA_DashAbility::OnBeforePlayMontage_Implementation(UPARAM(ref) UAnimMontage*& MontageToPlay, UPARAM(ref) FName& SectionName) {}

	bool UGA_DashAbility::ShouldEndOnHandoff_Implementation(const FGameplayEventData& Payload) const { return false; }

	void UGA_DashAbility::OnDashStarted_Implementation(FVector StartLocation, FVector ExpectedEndLocation) {}

	void UGA_DashAbility::OnDashEnded_Implementation(bool bWasCancelled) {}

	void UGA_DashAbility::OnDashHandoff_Implementation() {}

	void UGA_DashAbility::OnDashPassThrough_Implementation(FVector StartLocation, FVector EndLocation) {}

	void UGA_DashAbility::OnDashAttackWindowOpened_Implementation() {}

	void UGA_DashAbility::OnDashAttackWindowClosed_Implementation() {}

	bool UGA_DashAbility::OnDashAttackInput_Implementation(const FGameplayEventData& Payload) const { return false; }

	void UGA_DashAbility::OnProjectileDeflected_Implementation(AActor* Projectile) {}