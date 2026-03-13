// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StatComponent.h"
#include "EventBus.h"
#include "GameFramework/CharacterMovementComponent.h"


UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Subscribe to delegates
	if (UEventBus* Bus = UEventBus::Get())
	{
		Bus->OnStarvationStart.AddUObject(this, &UStatComponent::HandleStarvationStart);
		Bus->OnStarvationEnd.AddUObject(this, &UStatComponent::HandleStarvationEnd);
		Bus->OnDehydrationStart.AddUObject(this, &UStatComponent::HandleDehydrationStart);
		Bus->OnDehydrationEnd.AddUObject(this, &UStatComponent::HandleDehydrationEnd);
		Bus->OnLowBloodStart.AddUObject(this, &UStatComponent::HandleLowBloodStart);
		Bus->OnLowBloodEnd.AddUObject(this, &UStatComponent::HandleLowBloodEnd);
		Bus->OnFallingStart.AddUObject(this, &UStatComponent::HandleFallingStart);
		Bus->OnFallingEnd.AddUObject(this, &UStatComponent::HandleFallingEnd);
	}
}

void UStatComponent::TickStats(const float& DeltaTime)
{
	TickStamina(DeltaTime);
	TickHealth(DeltaTime);
	TickSatiation(DeltaTime);
	TickHydration(DeltaTime);
	TickBlood(DeltaTime);
}

void UStatComponent::TickStamina(const float& DeltaTime)
{
	// If dehydrated or starving, drain stamina
	if (bIsStarving || bIsDehydrated)
	{
		CurrentStaminaExhaustion  = 5.f;
		Stamina.Adjust(-1 * DeltaTime);
		if (!bIsExhausted)
		{
			bIsExhausted = true;
			if (const UEventBus* Bus = UEventBus::Get())
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
			if (const UEventBus* Bus= UEventBus::Get())
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
		if (const UEventBus* Bus = UEventBus::Get())
		{
			Bus->OnExhaustionEnd.Broadcast(GetOwner());
		}
	}
	Stamina.TickStat(DeltaTime);
}

void UStatComponent::TickSatiation(const float& DeltaTime)
{
	if (!bIsStarving && Satiation.GetCurrentValue() <= 0.f)
	{
		if (const UEventBus* Bus = UEventBus::Get())
		{
			Bus->OnStarvationStart.Broadcast(GetOwner());
		}
	}
	else if (bIsStarving && Satiation.GetCurrentValue() > 0.f)
	{
		if (const UEventBus* Bus = UEventBus::Get())
		{
			Bus->OnStarvationEnd.Broadcast(GetOwner());
		}
	}
	Satiation.TickStat(DeltaTime);
}

void UStatComponent::TickHydration(const float& DeltaTime)
{
	if (!bIsDehydrated && Hydration.GetCurrentValue() <= 0.f)
	{
		if (const UEventBus* Bus = UEventBus::Get())
		{
			Bus->OnDehydrationStart.Broadcast(GetOwner());
		}
	}
	else if (bIsDehydrated && Hydration.GetCurrentValue() > 0.f)
	{
		{
			if (const UEventBus* Bus = UEventBus::Get())
			{
				Bus->OnDehydrationEnd.Broadcast(GetOwner());
			}
		}
	}
	Hydration.TickStat(DeltaTime);
}

void UStatComponent::TickHealth(const float& DeltaTime)
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

void UStatComponent::TickBlood(const float& DeltaTime)
{
	if (bIsStarving || bIsDehydrated)
	{
		return;
	}
	if (!bHasLowBlood && Blood.GetCurrentValue() <= 40.f)
	{
		if (const UEventBus* Bus = UEventBus::Get())
		{
			Bus->OnLowBloodStart.Broadcast(GetOwner());
		}
	}
	else if (bHasLowBlood && Blood.GetCurrentValue() > 40.f)
	{
		if (const UEventBus* Bus = UEventBus::Get())
		{
			Bus->OnLowBloodEnd.Broadcast(GetOwner());
		}
	}
	Blood.TickStat(DeltaTime);
}

void UStatComponent::HandleStarvationStart(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bIsStarving = true;
}

void UStatComponent::HandleStarvationEnd(AActor* Actor)
{
	if(Actor != GetOwner())
	{
		return;
	}
	bIsStarving = false;
}

void UStatComponent::HandleDehydrationStart(AActor* Actor)
{
	if(Actor != GetOwner())
	{
		return;
	}
	bIsDehydrated = true;
}

void UStatComponent::HandleDehydrationEnd(AActor* Actor)
{
	if(Actor != GetOwner())
	{
		return;
	}
	bIsDehydrated = false;
}

void UStatComponent::HandleLowBloodStart(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bHasLowBlood = true;
}

void UStatComponent::HandleLowBloodEnd(AActor* Actor)
{
	if (Actor != GetOwner())
	{
		return;
	}
	bHasLowBlood = false;
}

void UStatComponent::HandleCrouchStart(AActor* Actor)
{
	bIsCrouching = true;
}

void UStatComponent::HandleCrouchEnd(AActor* Actor)
{
	bIsCrouching = false;
}

void UStatComponent::HandleFallingStart(AActor* Actor)
{
	bIsFalling = true;
}

void UStatComponent::HandleFallingEnd(AActor* Actor)
{
	bIsFalling = false;
}

void UStatComponent::SetSprint(bool bSprint)
{
	bIsSprinting = bSprint;
}

void UStatComponent::SetCrouch(bool bCrouch)
{
	bIsCrouching = bCrouch;
}

// Called every frame
void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TickType != ELevelTick::LEVELTICK_PauseTick)
	{
		TickStats(DeltaTime);
	}
}

float UStatComponent::GetStatPercentile(const EStatTypes Stat) const
{
	switch (Stat)
	{
	case EStatTypes::ST_HEALTH:
		return Health.Percentile();
	case EStatTypes::ST_STAMINA:
		return Stamina.Percentile();
	case EStatTypes::ST_SATIATION:
		return Satiation.Percentile();
	case EStatTypes::ST_HYDRATION:
		return Hydration.Percentile();
	case EStatTypes::ST_BLOOD:
		return Blood.Percentile();
	}
	return -1.f;
}

bool UStatComponent::CanJump()
{
	return !bIsExhausted && Stamina.GetCurrentValue() >= JumpCost;
}

void UStatComponent::ConsumeJumpStamina()
{
	Stamina.Adjust(0 - JumpCost);
}

void UStatComponent::AdjustStat(const EStatTypes Stat, const float& Amount)
{
	switch (Stat)
	{
	case EStatTypes::ST_HEALTH:
		Health.Adjust(Amount);
		return;
	case EStatTypes::ST_SATIATION:
		Satiation.Adjust(Amount);
		return;
	case EStatTypes::ST_HYDRATION:
		Hydration.Adjust(Amount);
		return;
	case EStatTypes::ST_STAMINA:
		Stamina.Adjust(Amount);
		return;
	case EStatTypes::ST_BLOOD:
		Blood.Adjust(Amount);
		return;
	}
	return;
}

