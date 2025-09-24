// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/TetheredPlayerController.h"

#include "AbilitySystem/TetheredAbilitySystemComponent.h"
#include "AbilitySystem/TetheredAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/TetheredPlayerCharacter.h"
#include "Debug/TetheredCheatManager.h"
#include "Engine/World.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/TetheredInputComponent.h"
#include "Math/RotationMatrix.h"
#include "TetheredGameplayTags.h"

ATetheredPlayerController::ATetheredPlayerController()
{
	// Set our custom cheat manager class
#if !UE_BUILD_SHIPPING
	CheatClass = UTetheredCheatManager::StaticClass();
#endif

	bReplicates = true;

	

	
}

void ATetheredPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

}


void ATetheredPlayerController::AbilityInputTagPressed(FGameplayTag Tag)
{
	if (UTetheredAbilitySystemComponent* ASC = GetASC())
		ASC->AbilityInputTagPressed(Tag);
}
void ATetheredPlayerController::AbilityInputTagReleased(FGameplayTag Tag)
{
	if (UTetheredAbilitySystemComponent* ASC = GetASC())
		ASC->AbilityInputTagReleased(Tag);
}
void ATetheredPlayerController::AbilityInputTagHeld(FGameplayTag Tag)
{
	if (UTetheredAbilitySystemComponent* ASC = GetASC())
		ASC->AbilityInputTagHeld(Tag);
}


UTetheredAbilitySystemComponent* ATetheredPlayerController::GetASC() const
{
	if (APawn* P = GetPawn())
		if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(P))
			return Cast<UTetheredAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	return nullptr;
}



void ATetheredPlayerController::BeginPlay()
{
	Super::BeginPlay();





	check(TetheredContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(TetheredContext, 0);
	}

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void ATetheredPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UTetheredInputComponent* TetheredInputComponent = CastChecked<UTetheredInputComponent>(InputComponent);

	// Ability Input Bindings
	TetheredInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATetheredPlayerController::Move);
	//TetheredInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATetheredPlayerController::Look);
	//TetheredInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);


	TetheredInputComponent->BindAbilityActions(InputConfig, this,
		&ThisClass::AbilityInputTagPressed,
		&ThisClass::AbilityInputTagReleased,
		&ThisClass::AbilityInputTagHeld);

}

void ATetheredPlayerController::Move(const FInputActionValue& InputActionValue)
{
	// Get movement input as 2D vector
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	
	// Only process movement if we have a valid pawn
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		// Check if movement is being overridden by an ability (like dash)
		if (ATetheredPlayerCharacter* TetheredCharacter = Cast<ATetheredPlayerCharacter>(ControlledPawn))
		{
			if (UTetheredAbilitySystemComponent* ASC = GetASC())
			{
				// Check if any movement-blocking abilities are active
				if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TetheredGameplayTags::TAG_State_Movement_Dashing.GetTag().GetTagName())))
				{
					return; // Don't process movement input during dash, etc.
				}
			}
		}
		
		// Apply camera-relative movement input
		ApplyMovementInput(InputAxisVector, ControlledPawn);
	}
}


void ATetheredPlayerController::ApplyMovementInput(const FVector2D& InputAxisVector, APawn* ControlledPawn)
{
	// Get camera rotation and calculate movement directions
	FVector ForwardDirection, RightDirection;
	GetCameraRelativeDirections(ForwardDirection, RightDirection);
	
	// Apply camera-relative movement input (same as your other project)
	ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
	ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
}

void ATetheredPlayerController::GetCameraRelativeDirections(FVector& OutForwardDirection, FVector& OutRightDirection)
{
	// Get camera rotation - try PlayerCameraManager first, fallback to ControlRotation
	FRotator CameraRotation;
	if (PlayerCameraManager)
	{
		CameraRotation = PlayerCameraManager->GetCameraRotation();
	}
	else
	{
		CameraRotation = GetControlRotation();
	}
	
	// Create rotation with only yaw component for ground movement
	const FRotator YawRotation(0, CameraRotation.Yaw, 0);
	
	// Calculate forward and right directions
	OutForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	OutRightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
}


void ATetheredPlayerController::SetRespawnTransform(const FTransform& NewRespawn)
{
	// save the new respawn transform
	RespawnTransform = NewRespawn;
}

void ATetheredPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// spawn a new character at the respawn transform
	if (ATetheredPlayerCharacter* RespawnedCharacter = GetWorld()->SpawnActor<ATetheredPlayerCharacter>(CharacterClass, RespawnTransform))
	{
		// possess the character
		Possess(RespawnedCharacter);
	}
}