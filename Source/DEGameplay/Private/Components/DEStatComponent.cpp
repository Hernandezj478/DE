// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/DEStatComponent.h"
#include "DEEventBus.h"
#include "GameFramework/CharacterMovementComponent.h"


UDEStatComponent::UDEStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UDEStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Subscribe to delegates
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnStarvationStart.AddUObject(this, &UDEStatComponent::HandleStarvationStart);
		Bus->OnStarvationEnd.AddUObject(this, &UDEStatComponent::HandleStarvationEnd);
		Bus->OnDehydrationStart.AddUObject(this, &UDEStatComponent::HandleDehydrationStart);
		Bus->OnDehydrationEnd.AddUObject(this, &UDEStatComponent::HandleDehydrationEnd);
		Bus->OnLowBloodStart.AddUObject(this, &UDEStatComponent::HandleLowBloodStart);
		Bus->OnLowBloodEnd.AddUObject(this, &UDEStatComponent::HandleLowBloodEnd);
		Bus->OnSprintStart.AddUObject(this, &UDEStatComponent::HandleSprintStart);
		Bus->OnSprintEnd.AddUObject(this, &UDEStatComponent::HandleSprintEnd);
		Bus->OnFallingStart.AddUObject(this, &UDEStatComponent::HandleFallingStart);
		Bus->OnFallingEnd.AddUObject(this, &UDEStatComponent::HandleFallingEnd);
	}
}

void UDEStatComponent::TickStats(const float& DeltaTime)
{
	TickStamina(DeltaTime);
	TickHealth(DeltaTime);
	TickSatiation(DeltaTime);
	TickHydration(DeltaTime);
	TickBlood(DeltaTime);
}

void UDEStatComponent::TickStamina(const float& DeltaTime)
{
	// If dehydrated or starving, drain stamina
	if (bIsStarving || bIsDehydrated)
	{
		CurrentStaminaExhaustion  = 5.f;
		Stamina.Adjust(-1 * DeltaTime);
		if (!bIsExhausted)
		{
			bIsExhausted = true;
			if (const UDEEventBus* Bus = UDEEventBus::Get())
			{
				Bus->OnExhaustionStart.Broadcast(GetOwner());
			}
		}
		return;
	}
	if (bIsFalling)
	{
		return;
	}
	// If exhausted, tick timer down and set exhausted state
	if (CurrentStaminaExhaustion > 0.0)
	{
		// Still exhausted
		if (!bIsExhausted)
		{
			bIsExhausted = true;
			if (const UDEEventBus* Bus= UDEEventBus::Get())
			{
				Bus->OnExhaustionStart.Broadcast(GetOwner());
			}
		}
		
		CurrentStaminaExhaustion -= DeltaTime;
		return;
	}
	// Sprint drain
	if (bIsSprinting)
	{
		Stamina.Adjust(0 - abs((DeltaTime * SprintCostMultiplier)));
		if (Stamina.GetCurrentValue() <= 0.0)
		{
			CurrentStaminaExhaustion = SecondsForStaminaExhaustion;
		}
		return;
	}
	// Recovery transition
	if (bIsExhausted && Stamina.GetCurrentValue() > 10.f)
	{
		bIsExhausted = false;
		if (const UDEEventBus* Bus = UDEEventBus::Get())
		{
			Bus->OnExhaustionEnd.Broadcast(GetOwner());
		}
	}
	Stamina.TickStat(DeltaTime);
}

void UDEStatComponent::TickSatiation(const float& DeltaTime)
{
	if (!bIsStarving && Satiation.GetCurrentValue() <= 0.f)
	{
		if (const UDEEventBus* Bus = UDEEventBus::Get())
		{
			Bus->OnStarvationStart.Broadcast(GetOwner());
		}
	}
	else if (bIsStarving && Satiation.GetCurrentValue() > 0.f)
	{
		if (const UDEEventBus* Bus = UDEEventBus::Get())
		{
			Bus->OnStarvationEnd.Broadcast(GetOwner());
		}
	}
	Satiation.TickStat(DeltaTime);
}

void UDEStatComponent::TickHydration(const float& DeltaTime)
{
	if (!bIsDehydrated && Hydration.GetCurrentValue() <= 0.f)
	{
		if (const UDEEventBus* Bus = UDEEventBus::Get())
		{
			Bus->OnDehydrationStart.Broadcast(GetOwner());
		}
	}
	else if (bIsDehydrated && Hydration.GetCurrentValue() > 0.f)
	{
		{
			if (const UDEEventBus* Bus = UDEEventBus::Get())
			{
				Bus->OnDehydrationEnd.Broadcast(GetOwner());
			}
		}
	}
	Hydration.TickStat(DeltaTime);
}

void UDEStatComponent::TickHealth(const float& DeltaTime)
{
	if (bHasLowBlood)
	{
		Health.Adjust(-5.f * DeltaTime);
		return;
	}
	if (bIsStarving || bIsDehydrated)
	{
		const float DrainRate = bIsStarving && bIsDehydrated ? 1.5f : 3.0f;
		
		Health.Adjust(-DrainRate * DeltaTime);
		return;
	}
	
	Health.TickStat(DeltaTime);
}

void UDEStatComponent::TickBlood(const float& DeltaTime)
{
	if (bIsStarving || bIsDehydrated)
	{
		return;
	}
	if (!bHasLowBlood && Blood.GetCurrentValue() <= 40.f)
	{
		if (const UDEEventBus* Bus = UDEEventBus::Get())
		{
			Bus->OnLowBloodStart.Broadcast(GetOwner());
		}
	}
	else if (bHasLowBlood && Blood.GetCurrentValue() > 40.f)
	{
		if (const UDEEventBus* Bus = UDEEventBus::Get())
		{
			Bus->OnLowBloodEnd.Broadcast(GetOwner());
		}
	}
	Blood.TickStat(DeltaTime);
}

void UDEStatComponent::HandleStarvationStart(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bIsStarving = true;
}

void UDEStatComponent::HandleStarvationEnd(AActor* Actor)
{
	if(Actor != GetOwner())
	{
		return;
	}
	bIsStarving = false;
}

void UDEStatComponent::HandleDehydrationStart(AActor* Actor)
{
	if(Actor != GetOwner())
	{
		return;
	}
	bIsDehydrated = true;
}

void UDEStatComponent::HandleDehydrationEnd(AActor* Actor)
{
	if(Actor != GetOwner())
	{
		return;
	}
	bIsDehydrated = false;
}

void UDEStatComponent::HandleLowBloodStart(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bHasLowBlood = true;
}

void UDEStatComponent::HandleLowBloodEnd(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bHasLowBlood = false;
}

void UDEStatComponent::HandleSprintStart(AActor* Actor)
{
	if (Actor == GetOwner())
	{
		bIsSprinting = true;
	}
}

void UDEStatComponent::HandleSprintEnd(AActor* Actor)
{
	if (Actor == GetOwner())
	{
		bIsSprinting = false;
	}
}

void UDEStatComponent::HandleCrouchStart(AActor* Actor)
{
	bIsCrouching = true;
}

void UDEStatComponent::HandleCrouchEnd(AActor* Actor)
{
	bIsCrouching = false;
}

void UDEStatComponent::HandleFallingStart(AActor* Actor)
{
	bIsFalling = true;
}

void UDEStatComponent::HandleFallingEnd(AActor* Actor)
{
	bIsFalling = false;
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
		return Hydration.Percentile();
	case EDEStatType::ST_BLOOD:
		return Blood.Percentile();
	}
	return -1.f;
}

bool UDEStatComponent::CanJump()
{
	return !bIsExhausted && Stamina.GetCurrentValue() >= JumpCost;
}

void UDEStatComponent::ConsumeJumpStamina()
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
	case EDEStatType::ST_BLOOD:
		Blood.Adjust(Amount);
		return;
	}
	return;
}

