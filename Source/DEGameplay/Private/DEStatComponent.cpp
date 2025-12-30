// Fill out your copyright notice in the Description page of Project Settings.

#include "DEStatComponent.h"
#include "DEEventBus.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UDEStatComponent::UDEStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UDEStatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningCharacterMovementComponent->MaxWalkSpeed = WalkSpeed;
}

void UDEStatComponent::TickStats(const float& DeltaTime)
{
	TickStamina(DeltaTime);
	TickHealth(DeltaTime);
	TickSatiation(DeltaTime);
	TickHydration(DeltaTime);
}

void UDEStatComponent::TickStamina(const float& DeltaTime)
{
	if (CurrentStaminaExhaustion > 0.0)
	{
		CurrentStaminaExhaustion -= DeltaTime;
		
		// Still exhausted
		if (!bIsExhausted)
		{
			bIsExhausted = true;
			if (const UDEEventBus* Bus= UDEEventBus::Get())
			{
				Bus->OnExhaustionStart.Broadcast(GetOwner());
			}
		}
		
		return;
	}
	// Sprint drain
	if (bIsSprinting && IsValidSprinting())
	{
		Stamina.Adjust(0 - abs((DeltaTime * SprintCostMultiplier)));
		if (Stamina.GetCurrentValue() <= 0.0)
		{
			SetSprinting(false);
			CurrentStaminaExhaustion = SecondsForStaminaExhaustion;
		}
		return;
	}
	Stamina.TickStat(DeltaTime);
	
	// Recovery transition
	if (bIsExhausted && Stamina.GetCurrentValue() > 0.f)
	{
		bIsExhausted = false;
		if (const UDEEventBus* Bus = UDEEventBus::Get())
		{
			Bus->OnExhaustionEnd.Broadcast(GetOwner());
		}
	}
}

void UDEStatComponent::TickSatiation(const float& DeltaTime)
{
	Satiation.TickStat(DeltaTime);
}

void UDEStatComponent::TickHydration(const float& DeltaTime)
{
	Hydration.TickStat(DeltaTime);
}

void UDEStatComponent::TickHealth(const float& DeltaTime)
{
	if (HealthRegenDelay > 0.0f)
	{
		HealthRegenDelay -= DeltaTime;
		return;
	}
	Health.TickStat(DeltaTime);
}

bool UDEStatComponent::IsValidSprinting()
{
	return OwningCharacterMovementComponent->Velocity.Length() > WalkSpeed && !OwningCharacterMovementComponent->IsFalling();
}

// Called every frame
void UDEStatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TickType != ELevelTick::LEVELTICK_PauseTick)
	{
		TickStats(DeltaTime);
	}
}

void UDEStatComponent::SetMovementComponentReference(UCharacterMovementComponent* Comp)
{
	OwningCharacterMovementComponent = Comp;
}

void UDEStatComponent::SetSneaking(const bool& IsSneaking)
{
	bIsSneaking = IsSneaking;
	if (bIsSprinting && !bIsSneaking)
	{
		return;
	}
	bIsSprinting = false;
	OwningCharacterMovementComponent->MaxWalkSpeed = bIsSneaking ? SneakSpeed : WalkSpeed;
}

float UDEStatComponent::GetStatPercentile(const EDEStatType Stat) const
{
	switch (Stat)
	{
	case EDEStatType::ST_HEALTH:
		return Health.Percentile();
	case EDEStatType::ST_STAMINA:
		return Stamina.Percentile();
	case EDEStatType::ST_SATIATION:
		return Satiation.Percentile();
	case EDEStatType::ST_HYDRATION:
		return Health.Percentile();
	}
	return -1.f;
}

bool UDEStatComponent::CanSprint() const
{
	return Stamina.GetCurrentValue() > 0.f;
}

void UDEStatComponent::SetSprinting(const bool& IsSprinting)
{
	bIsSprinting = IsSprinting;
	if (bIsSneaking && !bIsSprinting)
	{
		return;
	}
	bIsSneaking = false;
	OwningCharacterMovementComponent->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
}

bool UDEStatComponent::CanJump()
{
	return Stamina.GetCurrentValue() >= JumpCost;
}

void UDEStatComponent::HasJumped()
{
	Stamina.Adjust(0 - JumpCost);
}

void UDEStatComponent::AdjustStat(const EDEStatType Stat, const float& Amount)
{
	switch (Stat)
	{
	case EDEStatType::ST_HEALTH:
		Health.Adjust(Amount);
		return;
	case EDEStatType::ST_SATIATION:
		Satiation.Adjust(Amount);
		return;
	case EDEStatType::ST_HYDRATION:
		Hydration.Adjust(Amount);
		return;
	case EDEStatType::ST_STAMINA:
		Stamina.Adjust(Amount);
		return;
	}
	return;
}

