

#include "AbilitySystem/Abilities/TetheredGameplayAbility.h"
#include "AbilitySystem/TetheredAbilitySystemComponent.h"
#include "Player/TetheredPlayerController.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Character/TetheredCharacterBase.h"
#include "AbilitySystemLog.h"
//#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TetheredGameplayAbility)

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, "Ability.UserFacingSimpleActivateFail.Message");
UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, "Ability.PlayMontageOnActivateFail.Message");


UTetheredGameplayAbility::UTetheredGameplayAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;

	ActivationPolicy = ETetheredAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = ETetheredAbilityActivationGroup::Independent;

}

UTetheredAbilitySystemComponent* UTetheredGameplayAbility::GetTetheredAbilitySystemComponentFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<UTetheredAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr);
}

ATetheredPlayerController* UTetheredGameplayAbility::GetTetheredPlayerControllerFromActorInfo() const
{
	//return (CurrentActorInfo ? Cast<ATetheredPlayerController>(CurrentActorInfo->PlayerController.Get()) : nullptr);
	if (ATetheredPlayerController* PC = Cast<ATetheredPlayerController>(CurrentActorInfo->PlayerController.Get()))
	{
		return PC;
	}
	else
	{
		return nullptr;
	}
}

AController* UTetheredGameplayAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

ATetheredCharacterBase* UTetheredGameplayAbility::GetTetheredCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ATetheredCharacterBase>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

void UTetheredGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{

	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive()  && ActivationPolicy == ETetheredAbilityActivationPolicy::OnSpawn)

	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

bool UTetheredGameplayAbility::CanChangeActivationGroup(ETetheredAbilityActivationGroup NewGroup) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationGroup == NewGroup)
	{
		return true;
	}

	UTetheredAbilitySystemComponent* TetheredASC = GetTetheredAbilitySystemComponentFromActorInfo();
	check(TetheredASC);

	//if ((ActivationGroup != ETetheredAbilityActivationGroup::Exclusive_Blocking) && TetheredASC->IsActivationGroupBlocked(NewGroup))
	if (ActivationGroup != ETetheredAbilityActivationGroup::Exclusive_Blocking)
	{
		// This ability can't change groups if it's blocked (unless it is the one doing the blocking).
		return false;
	}

	if ((NewGroup == ETetheredAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
	{
		// This ability can't become replaceable if it can't be canceled.
		return false;
	}

	return true;
}

bool UTetheredGameplayAbility::ChangeActivationGroup(ETetheredAbilityActivationGroup NewGroup)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);

	if (!CanChangeActivationGroup(NewGroup))
	{
		return false;
	}

	//if (ActivationGroup != NewGroup)
	//{
	//	UTetheredAbilitySystemComponent* TetheredASC = GetTetheredAbilitySystemComponentFromActorInfo();
	//	check(TetheredASC);

	//	TetheredASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
	//	TetheredASC->AddAbilityToActivationGroup(NewGroup, this);

	//	ActivationGroup = NewGroup;
	//}

	return true;
}

void UTetheredGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	/*bool bSimpleFailureFound = false;
	for (FGameplayTag Reason : FailedReason)
	{
		if (!bSimpleFailureFound)
		{
			if (const FText* pUserFacingMessage = FailureTagToUserFacingMessages.Find(Reason))
			{
				FTetheredAbilitySimpleFailureMessage Message;
				Message.PlayerController = GetActorInfo().PlayerController.Get();
				Message.FailureTags = FailedReason;
				Message.UserFacingReason = *pUserFacingMessage;

				UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSystem.BroadcastMessage(TAG_ABILITY_SIMPLE_FAILURE_MESSAGE, Message);
				bSimpleFailureFound = true;
			}
		}

		if (UAnimMontage* pMontage = FailureTagToAnimMontage.FindRef(Reason))
		{
			FTetheredAbilityMontageFailureMessage Message;
			Message.PlayerController = GetActorInfo().PlayerController.Get();
			Message.AvatarActor = GetActorInfo().AvatarActor.Get();
			Message.FailureTags = FailedReason;
			Message.FailureMontage = pMontage;

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(TAG_ABILITY_PLAY_MONTAGE_FAILURE_MESSAGE, Message);
		}
	}*/
}

bool UTetheredGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	////@TODO Possibly remove after setting up tag relationships
	//UTetheredAbilitySystemComponent* TetheredASC = CastChecked<UTetheredAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	//if (TetheredASC->IsActivationGroupBlocked(ActivationGroup))
	//{
	//	if (OptionalRelevantTags)
	//	{
	//		OptionalRelevantTags->AddTag(TetheredGameplayTags::Ability_ActivateFail_ActivationGroup);
	//	}
	//	return false;
	//}

	return true;
}

void UTetheredGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	//// The ability can not block canceling if it's replaceable.
	//if (!bCanBeCanceled && (ActivationGroup == ETetheredAbilityActivationGroup::Exclusive_Replaceable))
	//{
	//	UE_LOG(LogTetheredAbilitySystem, Error, TEXT("SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable."), *GetName());
	//	return;
	//}

	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UTetheredGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UTetheredGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UTetheredGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UTetheredGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	//ClearCameraMode();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UTetheredGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags) || !ActorInfo)
	{
		return false;
	}

	//// Verify we can afford any additional costs
	//for (const TObjectPtr<UTetheredAbilityCost>& AdditionalCost : AdditionalCosts)
	//{
	//	if (AdditionalCost != nullptr)
	//	{
	//		if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, /*inout*/ OptionalRelevantTags))
	//		{
	//			return false;
	//		}
	//	}
	//}

	return true;
}

void UTetheredGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

//	check(ActorInfo);
//
//	// Used to determine if the ability actually hit a target (as some costs are only spent on successful attempts)
//	auto DetermineIfAbilityHitTarget = [&]()
//		{
//			if (ActorInfo->IsNetAuthority())
//			{
//				if (UTetheredAbilitySystemComponent* ASC = Cast<UTetheredAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
//				{
//					FGameplayAbilityTargetDataHandle TargetData;
//					ASC->GetAbilityTargetData(Handle, ActivationInfo, TargetData);
//					for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
//					{
//						if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
//						{
//							return true;
//						}
//					}
//				}
//			}
//
//			return false;
//		};
//
//	// Pay any additional costs
//	bool bAbilityHitTarget = false;
//	bool bHasDeterminedIfAbilityHitTarget = false;
//	for (const TObjectPtr<UTetheredAbilityCost>& AdditionalCost : AdditionalCosts)
//	{
//		if (AdditionalCost != nullptr)
//		{
//			if (AdditionalCost->ShouldOnlyApplyCostOnHit())
//			{
//				if (!bHasDeterminedIfAbilityHitTarget)
//				{
//					bAbilityHitTarget = DetermineIfAbilityHitTarget();
//					bHasDeterminedIfAbilityHitTarget = true;
//				}
//
//				if (!bAbilityHitTarget)
//				{
//					continue;
//				}
//			}
//
//			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
//		}
//	}

}
FGameplayEffectContextHandle UTetheredGameplayAbility::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	FGameplayEffectContextHandle ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);

	//FTetheredGameplayEffectContext* EffectContext = FTetheredGameplayEffectContext::ExtractEffectContext(ContextHandle);
	//check(EffectContext);

	//check(ActorInfo);

	//AActor* EffectCauser = nullptr;
	//const ITetheredAbilitySourceInterface* AbilitySource = nullptr;
	//float SourceLevel = 0.0f;
	//GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);

	//UObject* SourceObject = GetSourceObject(Handle, ActorInfo);

	//AActor* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

	//EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
	//EffectContext->AddInstigator(Instigator, EffectCauser);
	//EffectContext->AddSourceObject(SourceObject);

	return ContextHandle;
}

void UTetheredGameplayAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);

	//if (const FHitResult* HitResult = Spec.GetContext().GetHitResult())
	//{
	//	if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(HitResult->PhysMaterial.Get()))
	//	{
	//		Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
	//	}
	//}
}



void UTetheredGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}
