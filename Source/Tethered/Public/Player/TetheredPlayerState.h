// Copyright Nicholas Reardon

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "TetheredPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
/**
 *
 */
UCLASS()
class TETHERED_API ATetheredPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ATetheredPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; };
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	//FORCEINLINE int32 GetPlayerLevel() const { return Level; }
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;



private:
	//UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	//int32 Level = 1;

	//UFUNCTION()
	//void OnRep_Level(int32 OldLevel);
};
