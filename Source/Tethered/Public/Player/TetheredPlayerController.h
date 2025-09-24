// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "TetheredPlayerController.generated.h"

class UTetheredInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UTetheredAbilitySystemComponent;
class ATetheredPlayerCharacter;

/**
 * Custom Player Controller for Tethered combat game
 * Houses the player's Ability System Component and manages input mapping/routing
 * Routes movement input to character and ability inputs to ASC
 */
UCLASS()
class TETHERED_API ATetheredPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATetheredPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;


protected:
#pragma region Input Configuration
	/** Input mapping context for this player */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TObjectPtr<UInputMappingContext> TetheredContext;

	/** Input Mapping Contexts excluded on mobile */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;
#pragma endregion Input Configuration

#pragma region Input Actions
	/** Movement Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* MoveAction;

	///** Look Input Action */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	//UInputAction* LookAction;

	///** Jump Input Action */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	//UInputAction* JumpAction;

	/** Dash Input Action (Generic Movement Ability) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* DashAction;

	/** Light Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* LightAttackAction;

	/** Alternate Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* AlternateAttackAction;

	/** Special Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* SpecialAction;

	/** Run/Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Actions")
	UInputAction* RunAction;
#pragma endregion Input Actions

	void Move(const FInputActionValue& InputActionValue);

	/** Get camera-relative forward and right directions for movement */
	void GetCameraRelativeDirections(FVector& OutForwardDirection, FVector& OutRightDirection);

	/** Apply movement input with camera-relative directions */
	void ApplyMovementInput(const FVector2D& InputAxisVector, APawn* ControlledPawn);

	/** Apply movement speed from GAS to character movement component */
	void ApplyMovementSpeedFromGAS(UTetheredAbilitySystemComponent* ASC, APawn* ControlledPawn);

	/** Get movement speed from GAS attributes */
	float GetMovementSpeedFromGAS(UTetheredAbilitySystemComponent* ASC);

	//void CursorTrace();
	//TScriptInterface<IEnemyInterface> LastActor;
	//TScriptInterface<IEnemyInterface> ThisActor;
	//FHitResult CursorHit;

	//void ShiftPressed() { bShiftKeyDown = true; }
	//void ShiftReleased() { bShiftKeyDown = false; }
	//bool bShiftKeyDown = false;

	void AbilityInputTagPressed(const FGameplayTag InputTag);
	void AbilityInputTagReleased(const FGameplayTag InputTag);
	void AbilityInputTagHeld(const FGameplayTag InputTag);


	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UTetheredInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UTetheredAbilitySystemComponent> TetheredAbilitySystemComponent;

	UTetheredAbilitySystemComponent* GetASC() const;


public:

	/** Updates the character respawn transform */
	void SetRespawnTransform(const FTransform& NewRespawn);

protected:

	FTransform RespawnTransform;

	UPROPERTY(EditAnywhere, Category = "Respawn")
	TSubclassOf<ATetheredPlayerCharacter> CharacterClass;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

};