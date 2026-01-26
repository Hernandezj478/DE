// Fill out your copyright notice in the Description page of Project Settings.


#include "DECharacterBase.h"
#include "Components/DEMovementStateComponent.h"
#include "Components/DEStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DEInventoryComponent.h"


// Sets default values
ADECharacterBase::ADECharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Enable replication
	bReplicates = true;
	bAlwaysRelevant = true;
	GetCharacterMovement()->SetIsReplicated(true);
	GetMesh()->SetIsReplicated(true);
	Statline = CreateDefaultSubobject<UDEStatComponent>(TEXT("Statline"));
	MovementStateComponent = CreateDefaultSubobject<UDEMovementStateComponent>(TEXT("MovementStateComponent"));
}

// Called when the game starts or when spawned
void ADECharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

bool ADECharacterBase::CanCharacterJump() const
{
	return Statline->CanJump();
}

void ADECharacterBase::CharacterJump()
{
	Statline->ConsumeJumpStamina();
	Jump();
}

void ADECharacterBase::SetSprinting(const bool& bSprinting)
{
	MovementStateComponent->RequestSprint(bSprinting);
}

void ADECharacterBase::SetCrouch(const bool& bCrouch)
{
	MovementStateComponent->RequestCrouch(bCrouch);
}

// Called every frame
void ADECharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADECharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UActorComponent* ADECharacterBase::GetCharacterInventory() const
{
	return InventoryComponent;
}

