// Fill out your copyright notice in the Description page of Project Settings.


#include "DECharacterBase.h"
#include "Components/DEMovementStateComponent.h"
#include "DEStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


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
	Statline->SetMovementComponentReference(GetCharacterMovement());
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

bool ADECharacterBase::CanSprint() const
{
	return Statline->CanSprint();
}

void ADECharacterBase::HasJumped()
{
	Statline->HasJumped();
	ACharacter::Jump();
}

float ADECharacterBase::GetSneakSpeed() const
{
	return SneakSpeed;
}

float ADECharacterBase::GetWalkSpeed() const
{
	return WalkSpeed;
}

float ADECharacterBase::GetSprintSpeed() const
{
	return SprintSpeed;
}

void ADECharacterBase::SetSprinting(const bool& bSprinting)
{
	Statline->SetSprinting(bSprinting);
}

void ADECharacterBase::SetSneaking(const bool& bSneaking)
{
	Statline->SetSneaking(bSneaking);
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

