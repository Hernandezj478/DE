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
	SetIsReplicatedByDefault(true);
	CalculateTickRate();
}

// Called when the game starts
void UStatComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = GetOwner<ACharacterBase>();
	if (UMessagingSubsystem* MessagingSubsys = UMessagingSubsystem::Get())
	{
		pMessanger = MessagingSubsys;
	}
#if !UE_BUILD_SHIPPING
	if (!pMessanger)
	{
		LOG_CRITICAL(LogCharacters, "Missing Messaging Subsystem!");
	}
	if (!CharacterOwner)
	{
		LOG_CRITICAL(LogCharacters, "No Character Owner found for Stat Component!");
	}
#endif // !UE_BUILD_SHIPPING
}

void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (TickType == ELevelTick::LEVELTICK_PauseTick)
	{
		return;
	}
	TickStats(DeltaTime);
}

void UStatComponent::CalculateTickRate()
{
	int DayLengthInMinutes = 60; //EDayLenthToInt(Cast<UDEGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()))->GetDayLength());
	double SatiationTick = (double)1 / ((double)DayLengthInMinutes * (double)DaysToStarvation);
	double HydrationTick = (double)1 / ((double)DayLengthInMinutes * (double)DaysToDehydration);
	Satiation.AdjustTick(0 - SatiationTick);
	Hydration.AdjustTick(0 - HydrationTick);
}

void UStatComponent::TickStats(const float DeltaTime)
{
	TickStamina(DeltaTime);
	TickHealth(DeltaTime);
	TickBlood(DeltaTime);
	TickSatiation(DeltaTime);
	TickHydration(DeltaTime);
	TickFatigue(DeltaTime);
}

void UStatComponent::TickHealth(const float DeltaTime)
{
	if (CharacterOwner->HasAuthority())
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
}

void UStatComponent::TickStamina(const float DeltaTime)
{
	// Dehydration or starvation forces exhaustion regardless of stamina value
	bool bHasAuthority = CharacterOwner->HasAuthority();
	if (bHasAuthority)
	{
		if (bIsStarving || bIsDehydrated)
		{
			CurrentStaminaExhaustion = 5.f;
			Stamina.Adjust(0 - FMath::Abs((DeltaTime * StaminaExhaustedDrainRate)));
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
}

void UStatComponent::TickBlood(const float DeltaTime)
{
	if (CharacterOwner->HasAuthority())
	{
		if (bIsStarving || bIsDehydrated)
		{
			return;
		}
		if (!bHasLowBlood && Blood.GetCurrentValue() <= LowBloodThreshold)
		{
			bHasLowBlood = true;
			// TODO: Broadcast low blood message
		}
		else if (bHasLowBlood && Blood.GetCurrentValue() > LowBloodThreshold)
		{
			bHasLowBlood = false;
			// TODO: Braocast low blood message
		}
		Blood.TickStat(DeltaTime);
	}
}

void UStatComponent::TickSatiation(const float DeltaTime)
{
	if (CharacterOwner->HasAuthority())
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
}

void UStatComponent::TickHydration(const float DeltaTime)
{
	if (CharacterOwner->HasAuthority())
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
}

void UStatComponent::TickFatigue(const float DeltaTime)
{
	if (CharacterOwner->HasAuthority())
	{
		Fatigue.TickStat(DeltaTime);
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
	case EStatTypes::ST_BLOOD:
		return Blood.Percentile();
	case EStatTypes::ST_SATIATION:
		return Satiation.Percentile();
	case EStatTypes::ST_HYDRATION:
		return Hydration.Percentile();
	case EStatTypes::ST_FATIGUE:
		return Fatigue.Percentile();
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
	if (CharacterOwner->HasAuthority())
	{
		Stamina.Adjust(0 - JumpCost);
	}
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

void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UStatComponent, Health);
	DOREPLIFETIME(UStatComponent, Blood);
	DOREPLIFETIME_CONDITION(UStatComponent, Stamina, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UStatComponent, Satiation, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UStatComponent, Hydration, COND_OwnerOnly);
}

void UStatComponent::OnRep_Health()
{

}
void UStatComponent::OnRep_Stamina()
{
}
void UStatComponent::OnRep_Blood()
{
}
void UStatComponent::OnRep_Satiation()
{
}
void UStatComponent::OnRep_Hydration()
{
}
void UStatComponent::SetIsExhausted(bool bNewValue)
{
	if (bIsExhausted == bNewValue)
	{
		return;
	}
	bIsExhausted = bNewValue;
}

FName UStatComponent::GetSaveID_Implementation() const
{
	return FName("Statline");
}

bool UStatComponent::CollectSaveData_Implementation(FEntitySaveRecord& OutRecord) const
{
	FMemoryWriter Writer(OutRecord.CustomData);
	Writer << const_cast<FStat&>(Health);
	Writer << const_cast<FStat&>(Stamina);
	Writer << const_cast<FStat&>(Blood);
	Writer << const_cast<FStat&>(Satiation);
	Writer << const_cast<FStat&>(Hydration);
	Writer << const_cast<FStat&>(Fatigue);
	return true;
}

void UStatComponent::ApplySaveData_Implementation(const FEntitySaveRecord& Record)
{
	FMemoryReader Reader(Record.CustomData);
	Reader << Health;
	Reader << Stamina;
	Reader << Blood;
	Reader << Satiation;
	Reader << Hydration;
	Reader << Fatigue;
}

void UStatComponent::OnPreSave_Implementation()
{}

void UStatComponent::OnPostLoad_Implementation()
{}