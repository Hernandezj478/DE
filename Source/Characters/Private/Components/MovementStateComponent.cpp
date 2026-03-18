// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/MovementStateComponent.h"
#include "EventBus.h"
#include "CharacterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UMovementStateComponent::UMovementStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMovementStateComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		MovementComponent = Char->GetCharacterMovement();
		Char->MovementModeChangedDelegate.AddDynamic(this, &UMovementStateComponent::OnMovementModeChanged);
	}
	
	if (UEventBus* Bus = UEventBus::Get())
	{
		Bus->OnExhaustionStart.AddUObject(this, &UMovementStateComponent::HandleExhaustionStart);
		Bus->OnExhaustionEnd.AddUObject(this, &UMovementStateComponent::HandleExhaustionEnd);
	}
	ApplyWalkSpeed();
}

void UMovementStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UEventBus* Bus = UEventBus::Get())
	{
		Bus->OnExhaustionStart.RemoveAll(this);
		Bus->OnExhaustionEnd.RemoveAll(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UMovementStateComponent::EvaluateSprintState()
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

void UMovementStateComponent::EvaluateCrouchState()
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

void UMovementStateComponent::HandleExhaustionStart(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	SprintEnd();
	ApplyExhaustedSpeed();
	bCanSprint = false;
}

void UMovementStateComponent::HandleExhaustionEnd(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bCanSprint = true;
	ApplyWalkSpeed();
	EvaluateSprintState();
}

void UMovementStateComponent::ApplyWalkSpeed()
{
	MovementComponent->MaxWalkSpeed = WalkSpeed;
}

void UMovementStateComponent::ApplySprintSpeed()
{
	MovementComponent->MaxWalkSpeed = SprintSpeed;
}

void UMovementStateComponent::ApplyCrouchSpeed()
{
	MovementComponent->MaxWalkSpeed = CrouchSpeed;
}

void UMovementStateComponent::ApplyExhaustedSpeed()
{
	MovementComponent->MaxWalkSpeed = ExhaustedSpeed;
}

void UMovementStateComponent::OnMovementModeChanged(ACharacter* Character, EMovementMode NewMovementMode,
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

bool UMovementStateComponent::CanSprint() const
{
	return bCanSprint && !bIsFalling;
}

bool UMovementStateComponent::RequestSprint(const bool& bRequested)
{
	bSprintRequested = bRequested;
	EvaluateSprintState();
	return bIsSprinting;
}

bool UMovementStateComponent::RequestCrouch(const bool& bRequested)
{
	bCrouchRequested = bRequested;
	EvaluateCrouchState();
	return bIsCrouching;
}

void UMovementStateComponent::SprintStart()
{
	if (bIsSprinting)
	{
		return;
	}
	bIsSprinting = true;
	ApplySprintSpeed();
	// Might remove this call, for now we dont need this to communicate with statline to drain stamina
	// if (UEventBus* Bus = UEventBus::Get())
	// {
	// 	Bus->OnSprintStart.Broadcast(GetOwner());
	// }
}

void UMovementStateComponent::SprintEnd()
{
	if (!bIsSprinting)
	{
		return;
	}
	bIsSprinting = false;
	ApplyWalkSpeed();
	// if (UEventBus* Bus = UEventBus::Get())
	// {
	// 	Bus->OnSprintEnd.Broadcast(GetOwner());
	// }
}

void UMovementStateComponent::CrouchStart()
{
	if (bIsCrouching)
	{
		return;
	}
	bIsCrouching = true;
	ApplyCrouchSpeed();
	// if (UEventBus* Bus = UEventBus::Get())
	// {
	// 	Bus->OnCrouchStart.Broadcast(GetOwner());
	// }
}

void UMovementStateComponent::CrouchEnd()
{
	if (!bIsCrouching)
	{
		return;
	}
	bIsCrouching = false;
	ApplyWalkSpeed();
	EvaluateSprintState();
	// if (UEventBus* Bus = UEventBus::Get())
	// {
	// 	Bus->OnCrouchEnd.Broadcast(GetOwner());
	// }
}

void UMovementStateComponent::FallingStart()
{
	if (UEventBus* Bus = UEventBus::Get())
	{
		Bus->OnFallingStart.Broadcast(GetOwner());
	}
}

void UMovementStateComponent::FallingEnd()
{
	if (UEventBus* Bus = UEventBus::Get())
	{
		Bus->OnFallingEnd.Broadcast(GetOwner());
	}
}

float UMovementStateComponent::GetWalkSpeed() const
{
	return WalkSpeed;
}

