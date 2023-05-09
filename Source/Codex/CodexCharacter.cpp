// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodexCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimNotifyQueue.h"


//////////////////////////////////////////////////////////////////////////
// ACodexCharacter

ACodexCharacter::ACodexCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 230.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	KatanaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Katana"));
	KatanaMesh->SetupAttachment(GetMesh(),TEXT("Weapon_R"));
	
	AnimInstance = Cast<ACharacter>(this)->GetMesh()->GetAnimInstance();
	CurrentMontage = 0;
	CanMoveForAnimation = true;
}

void ACodexCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ACodexCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}


//////////////////////////////////////////////////////////////////////////
// Input

void ACodexCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACodexCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACodexCharacter::Look);

		//Equipe Katana
		EnhancedInputComponent->BindAction(EquipeKatana, ETriggerEvent::Started, this, &ACodexCharacter::EquipKatana);

		//Interaction
		EnhancedInputComponent->BindAction(Interact, ETriggerEvent::Started, this, &ACodexCharacter::Interaction);

		//Set Target
		EnhancedInputComponent->BindAction(SetTarget, ETriggerEvent::Started, this, &ACodexCharacter::SettingTarget);

		//Access Inventory
		EnhancedInputComponent->BindAction(AccessInventory, ETriggerEvent::Started, this, &ACodexCharacter::AccessingInventory);

		//Sprint
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Started, this, &ACodexCharacter::Sprinting);
		EnhancedInputComponent->BindAction(Sprint, ETriggerEvent::Completed, this, &ACodexCharacter::StopSprinting);

		//Defend
		EnhancedInputComponent->BindAction(Defense, ETriggerEvent::Triggered, this, &ACodexCharacter::Defending);

		//Attack
		EnhancedInputComponent->BindAction(Attack, ETriggerEvent::Started, this, &ACodexCharacter::Attacking);
	}

}

void ACodexCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	if(CanMoveForAnimation)
	{
		FVector2D MovementVector = Value.Get<FVector2D>();

		if (Controller != nullptr)
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
			// get right vector 
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// add movement 
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
	}	
}

void ACodexCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACodexCharacter::EquipKatana()
{ 
	IsKatanaEquiped = !IsKatanaEquiped;
}

void ACodexCharacter::Interaction()
{
	
}

void ACodexCharacter::SettingTarget()
{
	IsTargetSet = !IsTargetSet;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = !IsTargetSet;
}

void ACodexCharacter::AccessingInventory()
{
	
}

void ACodexCharacter::Sprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = 500.0f;
}

void ACodexCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed = 230.0f;
}

void ACodexCharacter::Attacking()
{
	if(CanMoveForAnimation)
	{
		CanMoveForAnimation = false;

		FVector Impluse = GetActorForwardVector();
		
		switch (CurrentMontage)
		{
		case 0 :
			PlayAnimMontage(Montage_1);
			LaunchCharacter(Impluse * 500, true, false);
			CurrentMontage++;
			break;
		case 1 :
			PlayAnimMontage(Montage_2);
			LaunchCharacter(Impluse * 500, true, false);
			CurrentMontage++;
			break;
		case 2 :
			PlayAnimMontage(Montage_3);
			LaunchCharacter(Impluse * 750, true, false);
			CurrentMontage++;
			break;
		case 3 :
			PlayAnimMontage(Montage_4);
			LaunchCharacter(Impluse * 1000, true, false);
			CurrentMontage = 0;
			break;
		default:
			CurrentMontage = 0;	
		}
	}	
}

void ACodexCharacter::Defending()
{
}

void ACodexCharacter::SetCanMoveOnAnimNotify()
{
	CanMoveForAnimation = true;
}







