// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/TetheredCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "UI/CombatLifeBar.h"
#include "Engine/DamageEvents.h"
#include "Controller/CombatPlayerController.h"

// Component includes
#include "Components/CombatComponent.h"
#include "Components/HealthComponent.h"
#include "Components/PlayerMovementComponent.h"


DEFINE_LOG_CATEGORY(LogTetheredCharacter);

ATetheredCharacter::ATetheredCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	// Configure character movement
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
	
	// IMPORTANT: Configure rotation settings for movement-based rotation
	bUseControllerRotationYaw = false; // Don't use controller rotation for character
	GetCharacterMovement()->bOrientRotationToMovement = false; // We'll handle rotation manually
	GetCharacterMovement()->bUseControllerDesiredRotation = false; // Don't use controller desired rotation

	// Create the camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = DefaultCameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;

	// Create the follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create the life bar widget component
	LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
	LifeBar->SetupAttachment(RootComponent);

	// Create the core system components
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	PlayerMovementComponent = CreateDefaultSubobject<UPlayerMovementComponent>(TEXT("PlayerMovementComponent"));

	// Set the player tag
	Tags.Add(FName("Player"));
}

void ATetheredCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the camera
	CameraBoom->TargetArmLength = DefaultCameraDistance;

	// Save the relative transform for the mesh so we can reset it after ragdoll animations
	MeshStartingTransform = GetMesh()->GetRelativeTransform();

	// Initialize all components
	if (CombatComponent)
	{
		CombatComponent->Initialize(this);
	}

	if (HealthComponent)
	{
		UCombatLifeBar* LifeBarWidget = Cast<UCombatLifeBar>(LifeBar->GetUserWidgetObject());
		HealthComponent->Initialize(this, LifeBarWidget);
	}

	if (PlayerMovementComponent)
	{
		PlayerMovementComponent->Initialize(this);
	}
}

void ATetheredCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATetheredCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Most tick logic is now handled by the individual components
}

void ATetheredCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Update the respawn transform on the Player Controller
	if (ACombatPlayerController* PC = Cast<ACombatPlayerController>(GetController()))
	{
		PC->SetRespawnTransform(GetActorTransform());
	}
}

void ATetheredCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Notify relevant components about landing
	if (HealthComponent && HealthComponent->IsAlive())
	{
		// Reset ragdoll physics through health component
		GetMesh()->SetPhysicsBlendWeight(0.0f);
	}
}

float ATetheredCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Route damage through health component
	if (HealthComponent)
	{
		// Health component will handle the actual damage processing
		return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	}
	
	return 0.0f;
}

#pragma region Input System
void ATetheredCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Movement - routes through PlayerMovementComponent
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATetheredCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATetheredCharacter::Look);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATetheredCharacter::Look);

		// Combat - routes through CombatComponent
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &ATetheredCharacter::ComboAttackPressed);
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Started, this, &ATetheredCharacter::ChargedAttackPressed);
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Completed, this, &ATetheredCharacter::ChargedAttackReleased);

		// Dash - routes through PlayerMovementComponent
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &ATetheredCharacter::Dash);

		// Run - routes through PlayerMovementComponent
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &ATetheredCharacter::RunPressed);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &ATetheredCharacter::RunReleased);
	}
}

void ATetheredCharacter::Move(const FInputActionValue& Value)
{
	// Route through PlayerMovementComponent for unified movement handling
	if (PlayerMovementComponent)
	{
		PlayerMovementComponent->HandleMovementInput(Value);
	}
}

void ATetheredCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ATetheredCharacter::DoLook(float Yaw, float Pitch)
{
	// Player will not manipulate the camera - this method intentionally left empty
	// The camera will follow the character automatically via the SpringArmComponent
}

void ATetheredCharacter::ComboAttackPressed()
{
	if (CombatComponent)
	{
		CombatComponent->DoComboAttackStart();
	}
}

void ATetheredCharacter::ChargedAttackPressed()
{
	if (CombatComponent)
	{
		CombatComponent->DoChargedAttackStart();
	}
}

void ATetheredCharacter::ChargedAttackReleased()
{
	if (CombatComponent)
	{
		CombatComponent->DoChargedAttackEnd();
	}
}

void ATetheredCharacter::Dash()
{
	// Route through PlayerMovementComponent for dash handling
	if (PlayerMovementComponent)
	{
		PlayerMovementComponent->HandleDashInput();
	}
}

void ATetheredCharacter::RunPressed()
{
	if (PlayerMovementComponent)
	{
		PlayerMovementComponent->SetRunState(true);
	}
}

void ATetheredCharacter::RunReleased()
{
	if (PlayerMovementComponent)
	{
		PlayerMovementComponent->SetRunState(false);
	}
}
#pragma endregion Input System

#pragma region ICombatAttacker Interface
void ATetheredCharacter::DoAttackTrace(FName DamageSourceBone)
{
	if (CombatComponent)
	{
		CombatComponent->DoAttackTrace(DamageSourceBone);
	}
}

void ATetheredCharacter::CheckCombo()
{
	if (CombatComponent)
	{
		CombatComponent->CheckCombo();
	}
}

void ATetheredCharacter::CheckChargedAttack()
{
	if (CombatComponent)
	{
		CombatComponent->CheckChargedAttack();
	}
}
#pragma endregion ICombatAttacker Interface

#pragma region ICombatDamageable Interface
void ATetheredCharacter::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	if (HealthComponent)
	{
		HealthComponent->ApplyDamage(Damage, DamageCauser, DamageLocation, DamageImpulse);
	}
	
	// Call Blueprint event directly on character
	OnReceivedDamage(Damage, DamageLocation, DamageCauser ? (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal() : FVector::ZeroVector);
}

void ATetheredCharacter::ApplyHealing(float Healing, AActor* Healer)
{
	if (HealthComponent)
	{
		HealthComponent->ApplyHealing(Healing, Healer);
	}
}

void ATetheredCharacter::HandleDeath()
{
	if (HealthComponent)
	{
		HealthComponent->HandleDeath();
	}
	
	// Call Blueprint event directly on character
	OnDeath();
}
#pragma endregion ICombatDamageable Interface