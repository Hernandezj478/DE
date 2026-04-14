// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementStateComponent.h"
#include "MessagingSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Logger.h"

UMovementStateComponent::UMovementStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	if (UMessagingSubsystem* MessagingSubsys = UMessagingSubsystem::Get())
	{
		pMessanger = MessagingSubsys;
	}
}

void UMovementStateComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		MovementComponent = Char->GetCharacterMovement();
		Char->MovementModeChangedDelegate.AddDynamic(this, &UMovementStateComponent::OnMovementModeChanged);
	}
	pMessanger->OnExhaustionChanged.AddDynamic(this, &UMovementStateComponent::OnExhaustionChanged);
	ApplyWalkSpeed();
}

void UMovementStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	pMessanger->OnExhaustionChanged.RemoveAll(this);
	Super::EndPlay(EndPlayReason);
}

void UMovementStateComponent::EvaluateSprintState()
{
	double vel = MovementComponent->GetVelocityForRVOConsideration().Length();
	if (vel == 0.0)
	{
		bIsSprinting = false;
		return;
	}
	if (bSprintRequested && CanSprint())
	{
		SprintStart();
	}
	else
	{
		SprintEnd();
	}
	//UpdateSprintState(bSprintRequested && CanSprint());
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
	//UpdateCrouchState(bCrouchRequested && !bIsSprinting);
}

void UMovementStateComponent::OnExhaustionChanged(bool IsExhausted)
{
	if (IsExhausted)
	{
		SprintEnd();
		//UpdateSprintState(false);
		ApplyExhaustedSpeed();
		bCanSprint = false;
		return;
	}
	// Reset to normal walking state
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
		//UpdateSprintState(false);
		FallingStart();
	}
	else
	{
		FallingEnd();
		EvaluateSprintState();
	}
}

void UMovementStateComponent::UpdateSprintState(bool bSprint)
{
	bIsSprinting = bSprint;
	bIsSprinting ? ApplySprintSpeed() : ApplyWalkSpeed();
	pMessanger->UpdateSprint(bIsSprinting);
}

void UMovementStateComponent::UpdateCrouchState(bool bCrouch)
{
	bIsCrouching = bCrouch;
	bIsCrouching ? ApplyCrouchSpeed() : ApplyWalkSpeed();
	pMessanger->UpdateCrouch(bIsCrouching);
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
	pMessanger->UpdateSprint(bIsSprinting);
}

void UMovementStateComponent::SprintEnd()
{
	if (!bIsSprinting)
	{
		return;
	}
	bIsSprinting = false;
	ApplyWalkSpeed();
	pMessanger->UpdateSprint(bIsSprinting);
}

void UMovementStateComponent::CrouchStart()
{
	if (bIsCrouching)
	{
		return;
	}
	bIsCrouching = true;
	ApplyCrouchSpeed();
	pMessanger->UpdateCrouch(bIsCrouching);
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
	pMessanger->UpdateCrouch(bIsCrouching);
}

void UMovementStateComponent::FallingStart()
{
	pMessanger->UpdateFalling(true);
}

void UMovementStateComponent::FallingEnd()
{
	pMessanger->UpdateFalling(false);
}

float UMovementStateComponent::GetWalkSpeed() const
{
	return WalkSpeed;
}

