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
#include "Components/AimAssistComponent.h"
#include "Data/AimAssistProfile.h"


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
	AimAssistComponent = CreateDefaultSubobject<UAimAssistComponent>(TEXT("AimAssistComponent"));

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

	// Initialize aim assist with default profile
	if (AimAssistComponent && DefaultAimAssistProfile)
	{
		AimAssistComponent->SetActiveProfile(DefaultAimAssistProfile);
	}
}

void ATetheredCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATetheredCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update aim assist with current input data
	UpdateAimAssistInput();
}

void ATetheredCharacter::UpdateAimAssistInput()
{
	if (!AimAssistComponent) return;

	// Convert movement input to aim direction
	FVector2D AimDirection = FVector2D::ZeroVector;
	

	const FVector ForwardVector = GetActorForwardVector();
	AimDirection = FVector2D(ForwardVector.X, ForwardVector.Y).GetSafeNormal();


	// Calculate input magnitude from both movement and look inputs
	const float MovementMagnitude = CurrentMovementInput.Size();
	const float LookMagnitude = CurrentLookInput.Size();
	const float CombinedMagnitude = FMath::Max(MovementMagnitude, LookMagnitude);
	
	AimAssistComponent->SetAimInputMagnitude(CombinedMagnitude);
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
	// Store movement input for aim assist
	FVector2D MovementVector = Value.Get<FVector2D>();
	CurrentMovementInput = MovementVector;

	// Route through PlayerMovementComponent for unified movement handling
	if (PlayerMovementComponent)
	{
		PlayerMovementComponent->HandleMovementInput(Value);
	}
}

void ATetheredCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	// Store look input for aim assist
	CurrentLookInput = LookAxisVector;
	
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
		// Get current target before combat starts
		AActor* CurrentTarget = nullptr;
		if (AimAssistComponent)
		{
			CurrentTarget = AimAssistComponent->GetCurrentTarget();
		}

		CombatComponent->DoComboAttackStart();
		
		// Trigger aim assist for melee attacks
		if (AimAssistComponent)
		{
			AimAssistComponent->OnMeleeCommit();
		}

		// Launch towards target if we have one (combo attack)
		if (CurrentTarget && PlayerMovementComponent)
		{
			LaunchTowardsTarget(CurrentTarget, false, ComboMaxLungeDistance);
		}
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
		// Get current target before combat starts
		AActor* CurrentTarget = nullptr;
		if (AimAssistComponent)
		{
			CurrentTarget = AimAssistComponent->GetCurrentTarget();
		}

		CombatComponent->DoChargedAttackEnd();
		
		// Trigger aim assist for charged melee attacks
		if (AimAssistComponent)
		{
			AimAssistComponent->OnMeleeCommit();
		}

		// Launch towards target for charged attacks (stronger and longer range)
		if (CurrentTarget && PlayerMovementComponent)
		{
			LaunchTowardsTarget(CurrentTarget, true, ChargedMaxLungeDistance);
		}
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

#pragma region Aim Assist Interface
void ATetheredCharacter::SetAimAssistProfile(UAimAssistProfile* NewProfile)
{
	if (AimAssistComponent)
	{
		AimAssistComponent->SetActiveProfile(NewProfile);
	}
	DefaultAimAssistProfile = NewProfile;
}

AActor* ATetheredCharacter::GetCurrentTarget() const
{
	if (AimAssistComponent)
	{
		return AimAssistComponent->GetCurrentTarget();
	}
	return nullptr;
}

void ATetheredCharacter::SetAimAssistEnabled(bool bEnabled)
{
	if (AimAssistComponent)
	{
		AimAssistComponent->SetComponentTickEnabled(bEnabled);
	}
}
#pragma endregion Aim Assist Interface

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

void ATetheredCharacter::LaunchTowardsTarget(AActor* Target, bool bIsChargedAttack, float MaxLungeDistance)
{
	if (!Target) return;

	// Use the provided MaxLungeDistance, or fall back to the appropriate default
	float EffectiveMaxDistance = MaxLungeDistance;
	if (EffectiveMaxDistance <= 0.0f)
	{
		EffectiveMaxDistance = bIsChargedAttack ? ChargedMaxLungeDistance : ComboMaxLungeDistance;
	}

	// Calculate direction and distance to target
	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector DirectionToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
	const float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

	// Don't launch if too close
	if (DistanceToTarget < MinLungeDistance)
	{
		UE_LOG(LogTetheredCharacter, Log, TEXT("Target %s too close (%.1f < %.1f), not launching"), 
			*Target->GetName(), DistanceToTarget, MinLungeDistance);
		return;
	}

	// Calculate the actual lunge distance (limited by max range)
	const float ActualLungeDistance = FMath::Min(DistanceToTarget, EffectiveMaxDistance);
	
	// Calculate target position for the lunge
	const FVector LungeTargetLocation = CurrentLocation + (DirectionToTarget * ActualLungeDistance);

	// Calculate launch velocity
	float LaunchSpeed = BaseLaunchSpeed;
	if (bIsChargedAttack)
	{
		LaunchSpeed *= ChargedLaunchMultiplier;
	}

	// Scale launch speed based on lunge distance ratio for better control
	const float DistanceRatio = FMath::Clamp(ActualLungeDistance / EffectiveMaxDistance, 0.3f, 1.0f);
	LaunchSpeed *= DistanceRatio;

	// Create launch velocity towards the calculated lunge target
	FVector LaunchVelocity = DirectionToTarget * LaunchSpeed;
	LaunchVelocity.Z = LaunchUpwardVelocity; // Configurable upward component

	// Launch the character
	LaunchCharacter(LaunchVelocity, true, true);

	// Log launch information
	if (DistanceToTarget > EffectiveMaxDistance)
	{
		UE_LOG(LogTetheredCharacter, Log, TEXT("Lunging towards %s: Distance %.1f > Max %.1f, lunging %.1f units"), 
			*Target->GetName(), DistanceToTarget, EffectiveMaxDistance, ActualLungeDistance);
	}
	else
	{
		UE_LOG(LogTetheredCharacter, Log, TEXT("Lunging towards %s: Distance %.1f, lunging full distance"), 
			*Target->GetName(), DistanceToTarget);
	}
}