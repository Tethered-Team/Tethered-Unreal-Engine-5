// Copyright Nicholas Reardon


#include "Player/TetheredPlayerState.h"

#include "AbilitySystem/TetheredAbilitySystemComponent.h"
#include "AbilitySystem/TetheredAttributeSet.h"
#include "Net/UnrealNetwork.h"

ATetheredPlayerState::ATetheredPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UTetheredAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UTetheredAttributeSet>("AttributeSet");


	SetNetUpdateFrequency(100.0f);


}

void ATetheredPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ATetheredPlayerState, Level);
}

//void AAuraPlayerState::OnRep_Level(int32 OldLevel)
//{
//
//}
