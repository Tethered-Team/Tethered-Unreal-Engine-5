// Copyright Nicholas Reardon


#include "AbilitySystem/TetheredAbilitySystemComponent.h"

#include "TetheredGameplayTags.h"
#include "AbilitySystem/Abilities/TetheredGameplayAbility.h"
#include "Tethered.h"
#include "AbilitySystem/TetheredAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TetheredAbilitySystemComponent)
DEFINE_LOG_CATEGORY(LogTetheredAbilitySystem);

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

UTetheredAbilitySystemComponent::UTetheredAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();

	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UTetheredAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UTetheredAbilitySystemComponent::ClientEffectApplied);


}

void UTetheredAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		// Notify all abilities that a new pawn avatar has been set
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
				ensureMsgf(AbilitySpec.Ability && AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("InitAbilityActorInfo: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
			PRAGMA_ENABLE_DEPRECATION_WARNINGS

				TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				UTetheredGameplayAbility* TetheredAbilityInstance = Cast<UTetheredGameplayAbility>(AbilityInstance);
				if (TetheredAbilityInstance)
				{
					// Ability instances may be missing for replays
					TetheredAbilityInstance->OnPawnAvatarSet();
				}
			}
		}

		//// Register with the global system once we actually have a pawn avatar. We wait until this time since some globally-applied effects may require an avatar.
		//if (UTetheredGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UTetheredGlobalAbilitySystem>(GetWorld()))
		//{
		//	GlobalAbilitySystem->RegisterASC(this);
		//}

		//if (UTetheredAnimInstance* TetheredAnimInst = Cast<UTetheredAnimInstance>(ActorInfo->GetAnimInstance()))
		//{
		//	TetheredAnimInst->InitializeWithAbilitySystem(this);
		//}

		TryActivateAbilitiesOnSpawn();
	}
}
void UTetheredAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UTetheredGameplayAbility* TetheredAbilityCDO = Cast<UTetheredGameplayAbility>(AbilitySpec.Ability);
		if (!TetheredAbilityCDO)
		{
			UE_LOG(LogTetheredAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Non-TetheredGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

			// Cancel all the spawned instances.
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			UTetheredGameplayAbility* TetheredAbilityInstance = CastChecked<UTetheredGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(TetheredAbilityInstance, AbilitySpec.Handle))
			{
				if (TetheredAbilityInstance->CanBeCanceled())
				{
					TetheredAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), TetheredAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
				}
				else
				{
					UE_LOG(LogTetheredAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *TetheredAbilityInstance->GetName());
				}
			}
		}
	}
}

void UTetheredAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UTetheredGameplayAbility* TetheredAbility, FGameplayAbilitySpecHandle Handle)
		{
			const ETetheredAbilityActivationPolicy ActivationPolicy = TetheredAbility->GetActivationPolicy();
			return ((ActivationPolicy == ETetheredAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == ETetheredAbilityActivationPolicy::WhileInputActive));
		};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}



void UTetheredAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UTetheredGameplayAbility* TetheredAbility = Cast<UTetheredGameplayAbility>(AbilitySpec.Ability))
		{
			FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
			DynamicTags.AddTag(TetheredAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
}




//void UTetheredAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
//{
//	if (!InputTag.IsValid()) return;
//
//	//TryActivateAbilitiesByTag(FGameplayTagContainer(InputTag)); // 
//	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
//	{
//		FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
//		//if (DynamicTags.HasAllExact(InputTags)) // For switching to container
//		if (DynamicTags.HasTagExact(InputTag))
//		{
//			AbilitySpecInputPressed(AbilitySpec);
//			if (!AbilitySpec.IsActive())
//			{
//				TryActivateAbility(AbilitySpec.Handle);
//			}
//		}
//	}
//}

void UTetheredAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UTetheredAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UTetheredAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	//@TODO: See if we can use FScopedServerAbilityRPCBatcher ScopedRPCBatcher in some of these loops

	//
	// Process all abilities that activate when the input is held.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UTetheredGameplayAbility* TetheredAbilityCDO = Cast<UTetheredGameplayAbility>(AbilitySpec->Ability);
				if (TetheredAbilityCDO && TetheredAbilityCDO->GetActivationPolicy() == ETetheredAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UTetheredGameplayAbility* TetheredAbilityCDO = Cast<UTetheredGameplayAbility>(AbilitySpec->Ability);

					if (TetheredAbilityCDO && TetheredAbilityCDO->GetActivationPolicy() == ETetheredAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	//
	// Clear the cached ability handles.
	//
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}
void UTetheredAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}


bool UTetheredAbilitySystemComponent::IsActivationGroupBlocked(ETetheredAbilityActivationGroup Group) const
{
	return false;
}

void UTetheredAbilitySystemComponent::AddAbilityToActivationGroup(ETetheredAbilityActivationGroup Group, UTetheredGameplayAbility* TetheredAbility)
{
	check(TetheredAbility);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case ETetheredAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case ETetheredAbilityActivationGroup::Exclusive_Replaceable:
	case ETetheredAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(ETetheredAbilityActivationGroup::Exclusive_Replaceable, TetheredAbility, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)ETetheredAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)ETetheredAbilityActivationGroup::Exclusive_Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogTetheredAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UTetheredAbilitySystemComponent::RemoveAbilityFromActivationGroup(ETetheredAbilityActivationGroup Group, UTetheredGameplayAbility* TetheredAbility)
{
	check(TetheredAbility);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void UTetheredAbilitySystemComponent::CancelActivationGroupAbilities(ETetheredAbilityActivationGroup Group, UTetheredGameplayAbility* IgnoreTetheredAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreTetheredAbility](const UTetheredGameplayAbility* TetheredAbility, FGameplayAbilitySpecHandle Handle)
		{
			return ((TetheredAbility->GetActivationGroup() == Group) && (TetheredAbility != IgnoreTetheredAbility));
		};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

//void UTetheredAbilitySystemComponent::AddDynamicTagGameplayEffect(const FGameplayTag& Tag)
//{
//	const TSubclassOf<UGameplayEffect> DynamicTagGE = UTetheredAssetManager::GetSubclass(UTetheredGameData::Get().DynamicTagGameplayEffect);
//	if (!DynamicTagGE)
//	{
//		UE_LOG(LogTetheredAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to find DynamicTagGameplayEffect [%s]."), *UTetheredGameData::Get().DynamicTagGameplayEffect.GetAssetName());
//		return;
//	}
//
//	const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(DynamicTagGE, 1.0f, MakeEffectContext());
//	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
//
//	if (!Spec)
//	{
//		UE_LOG(LogTetheredAbilitySystem, Warning, TEXT("AddDynamicTagGameplayEffect: Unable to make outgoing spec for [%s]."), *GetNameSafe(DynamicTagGE));
//		return;
//	}
//
//	Spec->DynamicGrantedTags.AddTag(Tag);
//
//	ApplyGameplayEffectSpecToSelf(*Spec);
//}

//void UTetheredAbilitySystemComponent::RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag)
//{
//	const TSubclassOf<UGameplayEffect> DynamicTagGE = UTetheredAssetManager::GetSubclass(UTetheredGameData::Get().DynamicTagGameplayEffect);
//	if (!DynamicTagGE)
//	{
//		UE_LOG(LogTetheredAbilitySystem, Warning, TEXT("RemoveDynamicTagGameplayEffect: Unable to find gameplay effect [%s]."), *UTetheredGameData::Get().DynamicTagGameplayEffect.GetAssetName());
//		return;
//	}
//
//	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
//	Query.EffectDefinition = DynamicTagGE;
//
//	RemoveActiveEffects(Query);
//}

void UTetheredAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

//void UTetheredAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
//{
//	if (TagRelationshipMapping)
//	{
//		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
//	}
//}

void UTetheredAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UTetheredGameplayAbility* TetheredAbilityCDO = Cast<UTetheredGameplayAbility>(AbilitySpec.Ability))
		{
			TetheredAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}


void UTetheredAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayEffectSpec& EffectSpec,
	FActiveGameplayEffectHandle ActiveHandle) const
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}



void UTetheredAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

			// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void UTetheredAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		FPredictionKey OriginalPredictionKey = Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

			// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

void UTetheredAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UTetheredGameplayAbility* TetheredAbility = Cast<UTetheredGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(TetheredAbility->GetActivationGroup(), TetheredAbility);
	}
}

void UTetheredAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UTetheredAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UTetheredGameplayAbility* TetheredAbility = Cast<UTetheredGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(TetheredAbility->GetActivationGroup(), TetheredAbility);
	}
}

void UTetheredAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	//if (TagRelationshipMapping)
	//{
	//	// Use the mapping to expand the ability tags into block and cancel tag
	//	TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	//}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: Apply any special logic like blocking input or movement
}

void UTetheredAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: Apply any special logic like blocking input or movement
}

void UTetheredAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void UTetheredAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	//UE_LOG(LogTetheredAbilitySystem, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

	if (const UTetheredGameplayAbility* TetheredAbility = Cast<const UTetheredGameplayAbility>(Ability))
	{
		TetheredAbility->OnAbilityFailedToActivate(FailureReason);
	}
}
