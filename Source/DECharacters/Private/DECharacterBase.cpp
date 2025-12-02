// Fill out your copyright notice in the Description page of Project Settings.


#include "DECharacterBase.h"

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
}

// Called when the game starts or when spawned
void ADECharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ADECharacterBase::CanCharacterJump() const
{
	return CanJump();
}

void ADECharacterBase::HasJumped()
{
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

void ADECharacterBase::SetSprinting(const bool bSprinting)
{
	if (bSprinting)
	{
		bIsSneaking = false;
		GetCharacterMovement()->MaxWalkSpeed = GetSprintSpeed();
		bIsSprinting = true;
		return;
	}
	if (bIsSneaking)
	{
		return;
	}
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = GetWalkSpeed();
	return;
}

void ADECharacterBase::SetSneaking(const bool bSneaking)
{
	if (bSneaking)
	{
		bIsSprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = GetSneakSpeed();
		bIsSneaking = true;
		return;
	}
	if (bIsSprinting)
	{
		return;
	}
	bIsSneaking = false;
	GetCharacterMovement()->MaxWalkSpeed = GetWalkSpeed();
	return;
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

