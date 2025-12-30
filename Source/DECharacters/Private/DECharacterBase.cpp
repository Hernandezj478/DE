// Fill out your copyright notice in the Description page of Project Settings.


#include "DECharacterBase.h"

#include "DEEventBus.h"
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
	
}

// Called when the game starts or when spawned
void ADECharacterBase::BeginPlay()
{
	Super::BeginPlay();
	// TODO: Move event listener to Movement Extension system later when implemented
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnExhaustionStart.AddUObject(this, &ADECharacterBase::HandleExhaustedStart);
		Bus->OnExhaustionEnd.AddUObject(this, &ADECharacterBase::HandleExhaustedEnd);
	}
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

void ADECharacterBase::HandleExhaustedStart(AActor* Actor)
{
	if (Actor != this)
	{
		return;
	}
	bCanSprint = false;
	bCanJump = false;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, 
			TEXT("Exhausted State Started"));
	}
}

void ADECharacterBase::HandleExhaustedEnd(AActor* Actor)
{
	if (Actor != this)
	{
		return;
	}
	bCanSprint = true;
	bCanJump = true;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
			TEXT("Exhausted State Ended"));
	}
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

