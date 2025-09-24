// Copyright Nicholas Reardon


#include "Character/TetheredCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/TetheredAbilitySystemComponent.h"
#include "Tethered.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/TetheredAttributeSet.h"

// Sets default values
ATetheredCharacterBase::ATetheredCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);


	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ATetheredCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	UTetheredAbilitySystemComponent* TetheredASC = Cast<UTetheredAbilitySystemComponent>(AbilitySystemComponent);
	const UTetheredAttributeSet* TetheredAttributeSet = AbilitySystemComponent->GetSet<UTetheredAttributeSet>();


	TetheredASC->GetGameplayAttributeValueChangeDelegate(TetheredAttributeSet->GetMovementSpeedAttribute()).AddUObject(this, &ATetheredCharacterBase::RecalcMaxSpeed);
	TetheredASC->GetGameplayAttributeValueChangeDelegate(TetheredAttributeSet->GetSprintMultiplierAttribute()).AddUObject(this, &ATetheredCharacterBase::RecalcMaxSpeed);


	RecalcMaxSpeed({});


}

void ATetheredCharacterBase::RecalcMaxSpeed(const FOnAttributeChangeData&)
{
	const UTetheredAttributeSet* TetheredAttributeSet = AbilitySystemComponent->GetSet<UTetheredAttributeSet>();
	const float BaseSpeed = TetheredAttributeSet->GetMovementSpeed();
	const float SprintMultiplier = TetheredAttributeSet->GetSprintMultiplier();
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * SprintMultiplier;
}

void ATetheredCharacterBase::InitAbilityActorInfo()
{

}

//FVector ATetheredCharacterBase::GetCombatSocketLocation() const
//{
//	DrawDebugSphere(GetWorld(), GetMesh()->GetSocketLocation(WeaponTipSocketName), 10.f, 12, FColor::Red, false, 5.f);
//	return Weapon->GetSocketLocation(WeaponTipSocketName);
//
//}


void ATetheredCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& GameplayEffectClass, const float Level) const
{
	check(GameplayEffectClass)
		UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC);
	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);
	ASC->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), ASC);

}

void ATetheredCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void ATetheredCharacterBase::AddCharacterAbilities()
{
	UTetheredAbilitySystemComponent* TetheredASC = Cast<UTetheredAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddCharacterAbilities can only be called on the server!"));
		return;
	}
	TetheredASC->AddCharacterAbilities(StartupAbilities);

	for (auto& Ability : TetheredASC->GetActivatableAbilities())
	{
		UE_LOG(LogTemp, Log, TEXT("Ability: %s"), *Ability.GetDebugString());
	}

	TArray<FGameplayAbilitySpecHandle> AbilityHandles;
	TetheredASC->GetAllAbilities(AbilityHandles);
	for (auto& Ability : AbilityHandles)
	{
		UE_LOG(LogTemp, Log, TEXT("Ability: %s"), *Ability.ToString());
	}


}
