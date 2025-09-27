// Copyright Nicholas Reardon


#include "Character/TetheredPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/TetheredAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/TetheredPlayerController.h"
#include "Player/TetheredPlayerState.h"
//#include "UI/HUD/TetheredHUD.h"

ATetheredPlayerCharacter::ATetheredPlayerCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;

}

void ATetheredPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	AddCharacterAbilities();
	BindMovementAttributes();
}

void ATetheredPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}


void ATetheredPlayerCharacter::InitAbilityActorInfo()
{
	// Init ability actor info for the server
	ATetheredPlayerState* TetheredPlayerState = GetPlayerState<ATetheredPlayerState>();
	check(TetheredPlayerState);
	TetheredPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(TetheredPlayerState, this);
	Cast<UTetheredAbilitySystemComponent>(TetheredPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	AbilitySystemComponent = TetheredPlayerState->GetAbilitySystemComponent();
	AttributeSet = TetheredPlayerState->GetAttributeSet();

	if (ATetheredPlayerController* TetheredPlayerController = GetController<ATetheredPlayerController>())
	{
		//if (ATetheredHUD* TetheredHUD = TetheredPlayerController->GetHUD<ATetheredHUD>())
		//{
		//	TetheredHUD->InitOverlay(TetheredPlayerController, TetheredPlayerState, AbilitySystemComponent, AttributeSet);
		//}
	}

	InitializeDefaultAttributes();
}

