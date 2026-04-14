// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/StatComponent.h"

#include "CharacterBase.h"
#include "Components/DECharacterMovementComponent.h"
#include "MessagingSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Logger.h"

UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	if (UMessagingSubsystem* MessagingSubsys = UMessagingSubsystem::Get())
	{
		pMessanger = MessagingSubsys;
	}
#if !UE_BUILD_SHIPPING
	if (!pMessanger)
	{
		Logger::GetInstance()->AddMessage("Messaging Subsystem not found!", CRITICAL);
	}
#endif // !UE_BUILD_SHIPPING
}

// Called when the game starts
void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ACharacterBase>(GetOwner());
	PredictedStamina = Stamina.GetCurrentValue();
}
// =========================================================
// Tick Stats
// =========================================================
void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!GetOwner()->HasAuthority() || TickType == ELevelTick::LEVELTICK_PauseTick)
	{
		return;
	}
	switch (GetOwner()->GetLocalRole())
	{
	case ROLE_Authority:
		// Server runs full authoritive simulation
		TickStats(DeltaTime);
		break;
	case ROLE_AutonomousProxy:
		// Local player predicts stamina for UI responsiveness
		// Runs local-only stats since nobody else needs them
		TickStaminaPredicted(DeltaTime);
		TickSatiation(DeltaTime);
		TickHydration(DeltaTime);
		break;
	default:
		// SimulatedProxy - display replicated state only, tick nothing
		break;
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
// =========================================================
// Server Tick
// =========================================================
void UStatComponent::TickStamina(const float& DeltaTime)
{
	// Dehydration or starvation forces exhaustion regardless of stamina value
	if (bIsStarving || bIsDehydrated)
	{
		CurrentStaminaExhaustion  = 5.f;
		Stamina.Adjust(-1 * DeltaTime);
		SetIsExhausted(true);
		return;
	}
	if (CharacterOwner->IsFalling())
	{
		return;
	}
	// Still in exhaustion recovery window
	if (CurrentStaminaExhaustion > 0.0)
	{
		// Still exhausted
		SetIsExhausted(true);
		CurrentStaminaExhaustion -= DeltaTime;
		return;
	}
	// Sprint drain
	if (CharacterOwner->IsSprinting())
	{
		Stamina.Adjust(0 - FMath::Abs((DeltaTime * SprintCostMultiplier)));
		if (Stamina.GetCurrentValue() <= 0.0)
		{
			CurrentStaminaExhaustion = SecondsForStaminaExhaustion;
		}
		return;
	}
	// Recovery transition
	if (bIsExhausted && Stamina.GetCurrentValue() >= ExhaustionThreashold)
	{
		SetIsExhausted(false);
	}
	Stamina.TickStat(DeltaTime);
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
		const float DrainRate = (bIsStarving && bIsDehydrated) ? MaxHealthDrainRate : HealthDrainRate;

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
		bHasLowBlood = true;
		// TODO: Broadcast low blood message
	}
	else if (bHasLowBlood && Blood.GetCurrentValue() > 40.f)
	{
		bHasLowBlood = false;
		// TODO: Braocast low blood message
	}
	Blood.TickStat(DeltaTime);
}

void UStatComponent::TickSatiation(const float& DeltaTime)
{
	if (!bIsStarving && Satiation.GetCurrentValue() <= 0.f)
	{
		bIsStarving = true;
		// TODO: Bradcast starvation message
	}
	else if (bIsStarving && Satiation.GetCurrentValue() > 0.f)
	{
		bIsStarving = false;
		// TODO: Broadcast starvation message
	}
	Satiation.TickStat(DeltaTime);
}

void UStatComponent::TickHydration(const float& DeltaTime)
{
	if (!bIsDehydrated && Hydration.GetCurrentValue() <= 0.f)
	{
		bIsDehydrated = true;
		// TODO: Broadcast dehydration message
		
	}
	else if (bIsDehydrated && Hydration.GetCurrentValue() > 0.f)
	{
		bIsDehydrated = false;
		// TODO: Broadcast dehydration message
	}
	Hydration.TickStat(DeltaTime);
}
// =========================================================
// Client Prediction
// =========================================================
void UStatComponent::TickStaminaPredicted(float DeltaTime)
{
	// Mirrors server drain logic locally - drives UI only, never gates actions
	if (CharacterOwner->IsSprinting())
	{
		PredictedStamina = FMath::Max(0.f, PredictedStamina - FMath::Abs(DeltaTime * SprintCostMultiplier));
	}
	else
	{
		PredictedStamina = FMath::Min(Stamina.GetMaxValue(), PredictedStamina + DeltaTime * Stamina.GetTickRate());
	}
	// TODO: Broadcast stamina UI
	// pMessanger->UpdateStaminaUI(PredictedStamina / Stamina.GetMaxValue());
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

bool UStatComponent::CanSprint()
{
	return !bIsExhausted/* && Stamina.GetCurrentValue() >= 5*/;
}

bool UStatComponent::CanRun()
{
	return !bIsExhausted;
}

void UStatComponent::ConsumeJumpStamina()
{
	Stamina.Adjust(0 - JumpCost);
}
// =========================================================
// Public Interface
// =========================================================
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
// =========================================================
// Lifetime Props
// =========================================================
void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Health to all - players and NPCs interact with health
	DOREPLIFETIME(UStatComponent, Health);
	DOREPLIFETIME(UStatComponent, Blood);

	// Stamina to owner only - server corrects client prediction
	DOREPLIFETIME_CONDITION(UStatComponent, Stamina, COND_OwnerOnly);
	
	// Exhaustion to all - SimulatedProxies need it to gate visible movement
	DOREPLIFETIME(UStatComponent, bIsExhausted);
}
// =========================================================
// Rep Notifies
// =========================================================
void UStatComponent::OnRep_Stamina()
{
	if (pMessanger)
	{
		pMessanger->UpdateStamina(Stamina.Percentile());
	}
}

void UStatComponent::OnRep_Blood()
{
	if (pMessanger)
	{
		pMessanger->UpdateBlood(Blood.Percentile());
	}
}

void UStatComponent::OnRep_IsExhausted()
{
	if (pMessanger)
	{
		pMessanger->UpdateExhustion(bIsExhausted);
	}
}
// =========================================================
// State Setters
// =========================================================
void UStatComponent::SetIsExhausted(bool bNewValue)
{
	if (bIsExhausted == bNewValue)
	{
		return;
	}
	bIsExhausted = bNewValue;
	OnRep_IsExhausted();
}
