// Copyright Nicholas Reardon

#pragma once

#include "CoreMinimal.h"
#include "Character/TetheredCharacterBase.h"
#include "Player/TetheredPlayerState.h"
#include "TetheredPlayerCharacter.generated.h"

/**
 *
 */
UCLASS()
class TETHERED_API ATetheredPlayerCharacter : public ATetheredCharacterBase
{
	GENERATED_BODY()

public:
	ATetheredPlayerCharacter();
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

private:
	virtual void InitAbilityActorInfo() override;
};
