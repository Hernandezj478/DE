// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Logger.h"


// Sets default values
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = false;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	// Don't rotate when controller rotates. Affect only camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	// Create camera boom, pulls in when collision detected
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	
	// Create 3rd person camera
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;
	ThirdPersonCamera->Deactivate();
	ThirdPersonCamera->bAutoActivate = false;
	
	// Create 1st person mesh component
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	check(FirstPersonMeshComponent != nullptr);
	
	// Configure mesh components
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	GetMesh()->SetOwnerNoSee(true);
	FirstPersonMeshComponent->SetupAttachment(GetMesh());
	FirstPersonMeshComponent->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	FirstPersonMeshComponent->SetCollisionProfileName(FName("NoCollision"));
	
	// Create 1st person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(FirstPersonMeshComponent, FName("head"));
	
	FirstPersonCamera->SetRelativeLocationAndRotation(FirstPersonCameraOffset, FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	// Enable first-person rendering on the camera and set default FOV and scale values
	FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
	FirstPersonCamera->bEnableFirstPersonScale = true;
	FirstPersonCamera->FirstPersonFieldOfView = FirstPersonFieldOfView;
	FirstPersonCamera->FirstPersonScale = FirstPersonScale;
	
	// Enable first-person camera by default
	FirstPersonCamera->Activate();
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Get the player controller for this character
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2d>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0, Rotation.Yaw, 0.0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y).GetSafeNormal();
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::TogglePerspective()
{
	bInFirstPerson = !bInFirstPerson;
	if (!bInFirstPerson)
	{
		FirstPersonCamera->Deactivate();
		ThirdPersonCamera->Activate();
		bUseControllerRotationPitch = false;
		bUseControllerRotationYaw = false;
		bUseControllerRotationRoll = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetMesh()->SetOwnerNoSee(false);
		GetMesh()->SetCollisionProfileName(FName("Collision"));
		FirstPersonMeshComponent->SetOnlyOwnerSee(false);
		return;
	}
	ThirdPersonCamera->Deactivate();
	FirstPersonCamera->Activate();
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	return;
}

void APlayerCharacter::ToggleWalkRun()
{
	if (!bIsRunning)
	{
		Logger::GetInstance()->AddMessage("ToggleWalkRun - Run", DEBUG);
		Run();
		return;
	}
	Logger::GetInstance()->AddMessage("ToggleWalkRun - StopRun", DEBUG);
	StopRun();
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(ToggleCameraPerspective, ETriggerEvent::Completed, this, &APlayerCharacter::TogglePerspective);
		EnhancedInputComponent->BindAction(ToggleRun, ETriggerEvent::Completed, this, &APlayerCharacter::ToggleWalkRun);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ACharacterBase::Crouch, false);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ACharacterBase::UnCrouch, false);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ACharacterBase::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACharacterBase::StopSprint);
		

		// EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Completed, this, &APlayerCharacter::Interaction);
		// EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Completed, this, &APlayerCharacter::Inventory);
		// EnhancedInputComponent->BindAction(LeanAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Lean);
		// EnhancedInputComponent->BindAction(LeanAction, ETriggerEvent::Completed, this, &APlayerCharacter::Lean);
	}
}

