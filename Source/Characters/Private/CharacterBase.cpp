// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "MovementStateComponent.h"
#include "StatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InventoryComponent.h"
#include "Logger.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Enable replication
	bReplicates = true;
	bAlwaysRelevant = true;
	GetCharacterMovement()->SetIsReplicated(true);
	GetMesh()->SetIsReplicated(true);
	Statline = CreateDefaultSubobject<UStatComponent>(TEXT("Statline"));
	if (!IsValid(Statline))
	{
		Logger::GetInstance()->AddMessage("Statline has not been created/initialized", ERROR);
	}
	MovementStateComponent = CreateDefaultSubobject<UMovementStateComponent>(TEXT("MovementStateComponent"));
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

bool ACharacterBase::CanCharacterJump() const
{
	return Statline->CanJump();
}

void ACharacterBase::CharacterJump()
{
	Statline->ConsumeJumpStamina();
	Jump();
}

void ACharacterBase::SetSprinting(const bool& bSprinting)
{
	Statline->SetSprint(MovementStateComponent->RequestSprint(bSprinting));
}

void ACharacterBase::SetCrouch(const bool& bCrouch)
{
	Statline->SetCrouch(MovementStateComponent->RequestCrouch(bCrouch));
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UActorComponent* ACharacterBase::GetCharacterInventory() const
{
	return InventoryComponent;
}

