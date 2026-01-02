// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DEMovementStateComponent.h"
#include "DEEventBus.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDEMovementStateComponent::UDEMovementStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDEMovementStateComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		MovementComponent = Char->GetCharacterMovement();
		Char->MovementModeChangedDelegate.AddDynamic(this, &UDEMovementStateComponent::OnMovementModeChanged);
	}
	
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnExhaustionStart.AddUObject(this, &UDEMovementStateComponent::HandleExhaustionStart);
		Bus->OnExhaustionEnd.AddUObject(this, &UDEMovementStateComponent::HandleExhaustionEnd);
	}
	ApplyWalkSpeed();
}

void UDEMovementStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnExhaustionStart.RemoveAll(this);
		Bus->OnExhaustionEnd.RemoveAll(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UDEMovementStateComponent::EvaluateSprintState()
{
	if (bSprintRequested && CanSprint())
	{
		SprintStart();
	}
	else
	{
		SprintEnd();
	}
}

void UDEMovementStateComponent::EvaluateCrouchState()
{
	if (bCrouchRequested && !bIsSprinting)
	{
		CrouchStart();
	}
	else
	{
		CrouchEnd();
	}
}

void UDEMovementStateComponent::HandleExhaustionStart(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	SprintEnd();
	ApplyExhaustedSpeed();
	bCanSprint = false;
}

void UDEMovementStateComponent::HandleExhaustionEnd(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bCanSprint = true;
	ApplyWalkSpeed();
	EvaluateSprintState();
}

void UDEMovementStateComponent::ApplyWalkSpeed()
{
	MovementComponent->MaxWalkSpeed = WalkSpeed;
}

void UDEMovementStateComponent::ApplySprintSpeed()
{
	MovementComponent->MaxWalkSpeed = SprintSpeed;
}

void UDEMovementStateComponent::ApplyCrouchSpeed()
{
	MovementComponent->MaxWalkSpeed = CrouchSpeed;
}

void UDEMovementStateComponent::ApplyExhaustedSpeed()
{
	MovementComponent->MaxWalkSpeed = ExhaustedSpeed;
}

void UDEMovementStateComponent::OnMovementModeChanged(ACharacter* Character, EMovementMode NewMovementMode,
                                                      uint8 PreviousCustomMode)
{
	bIsFalling = Character->GetCharacterMovement()->IsFalling();
	if (bIsFalling)
	{
		SprintEnd();
		FallingStart();	
	}
	else
	{
		EvaluateSprintState();
		FallingEnd();
	}
}

bool UDEMovementStateComponent::CanSprint() const
{
	return bCanSprint && !bIsFalling;
}

void UDEMovementStateComponent::RequestSprint(const bool& bRequested)
{
	bSprintRequested = bRequested;
	EvaluateSprintState();
}

void UDEMovementStateComponent::RequestCrouch(const bool& bRequested)
{
	bCrouchRequested = bRequested;
	EvaluateCrouchState();
}

void UDEMovementStateComponent::SprintStart()
{
	if (bIsSprinting)
	{
		return;
	}
	bIsSprinting = true;
	ApplySprintSpeed();
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnSprintStart.Broadcast(GetOwner());
	}
}

void UDEMovementStateComponent::SprintEnd()
{
	if (!bIsSprinting)
	{
		return;
	}
	bIsSprinting = false;
	ApplyWalkSpeed();
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnSprintEnd.Broadcast(GetOwner());
	}
}

void UDEMovementStateComponent::CrouchStart()
{
	if (bIsCrouching)
	{
		return;
	}
	bIsCrouching = true;
	ApplyCrouchSpeed();
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnCrouchStart.Broadcast(GetOwner());
	}
}

void UDEMovementStateComponent::CrouchEnd()
{
	if (!bIsCrouching)
	{
		return;
	}
	bIsCrouching = false;
	ApplyWalkSpeed();
	EvaluateSprintState();
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnCrouchEnd.Broadcast(GetOwner());
	}
}

void UDEMovementStateComponent::FallingStart()
{
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnFallingStart.Broadcast(GetOwner());
	}
}

void UDEMovementStateComponent::FallingEnd()
{
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnFallingEnd.Broadcast(GetOwner());
	}
}

float UDEMovementStateComponent::GetWalkSpeed() const
{
	return WalkSpeed;
}

