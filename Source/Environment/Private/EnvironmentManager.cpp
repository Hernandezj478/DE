// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"
#include "MessagingSubsystem.h"
#include "DEWorldSettings.h"
#include "WeatherTransitionData.h"
#include "WeatherTransition.h"
#include "Logger.h"

void UEnvironmentManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
	LOG_DEBUG(LogEnvironment, "Initialized Environment");
#endif
	if (UWorld* pWorld = GetWorld())
	{
		FString MapName = pWorld->GetMapName();
		MapName = pWorld->RemovePIEPrefix(MapName);
		if (MapName.Equals("MainMenu", ESearchCase::IgnoreCase))
		{
#if !UE_BUILD_SHIPPING
			LOG_DEBUG(LogEnvironment, "Main Menu");
#endif
			bCanEverTick = false;
		}
		SetTickableTickType(ETickableTickType::Conditional);
	}
	if (UMessagingSubsystem* MessagingSubsystem = UMessagingSubsystem::Get())
	{
		pMessanger = MessagingSubsystem;
	}
}

void UEnvironmentManager::Deinitialize()
{
	Super::Deinitialize();
}

void UEnvironmentManager::Tick(float DeltaTime)
{
	// Server world drives environment state
	// Clients receive replicated data
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}
	UpdateTime(DeltaTime);
	if (bHasTemperatureData)
	{
		UpdateTemperature(DeltaTime);
	}
	if (bTimeWasUpdated)
	{
		pMessanger->UpdateTime(CurrentTime.DayOfYear, CurrentTime.Year, CurrentTime.Month, 
			CurrentTime.DayOfMonth, CurrentTime.Hour, CurrentTime.Minute);
		bTimeWasUpdated = false;
	}
}

bool UEnvironmentManager::IsTickable() const
{
	if (!HasAllFlags(RF_ClassDefaultObject))
	{
		return bCanEverTick;
	}
	return false;
}

UWorld* UEnvironmentManager::GetTickableGameObjectWorld() const
{
	return Super::GetTickableGameObjectWorld();
}

TStatId UEnvironmentManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UEnvironmentManager, STATGROUP_Tickables);
}

void UEnvironmentManager::OnWorldBeginPlay(UWorld& InWorld)
{
	// Clients dont need to load curves, they receive replicated state from server
	Super::OnWorldBeginPlay(InWorld);
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	CalculateDayLength();
	SetDayOfYear();
	pWorldSettings = Cast<ADEWorldSettings>(GetWorld()->GetWorldSettings());
	if (IsValid(pWorldSettings))
	{
		if (!pWorldSettings->DailyTemperatureRange.IsNull())
		{
			DailyTemperatureRange = pWorldSettings->DailyTemperatureRange.LoadSynchronous();
		}
		if (!pWorldSettings->AnnualTemperatureRange.IsNull())
		{
			AnnualTemperatureRange = pWorldSettings->AnnualTemperatureRange.LoadSynchronous();
		}
		if (!pWorldSettings->WorldWeatherData.IsNull())
		{
			pWeatherData = Cast<UWeatherTransitionData>(pWorldSettings->WorldWeatherData.LoadSynchronous());
			bHasWeatherData = true;
			SelectNextWeatherState();
		}
	}
	if (!IsValid(DailyTemperatureRange) && !IsValid(AnnualTemperatureRange))
	{
		LOG_WARNING(LogEnvironment, "Daily/Annual Temperature Range not valid!");
		bHasTemperatureData = false;
	}
}

void UEnvironmentManager::UpdateTime(float DeltaTime)
{
	TimeDecay -= DeltaTime;
	if (TimeDecay <= 0.0f)
	{
		TimeDecay += MinuteLength;
		AdvanceMinute();
	}
}

void UEnvironmentManager::AdvanceMinute()
{
	bTimeWasUpdated = true;
	CurrentTime.Minute++;
	if (CurrentTime.Minute > 59)
	{
		CurrentTime.Minute = 0;
		AdvanceHour();
	}
	pMessanger->UpdateMinute(CurrentTime.Minute);
}

void UEnvironmentManager::AdvanceHour()
{
	bTimeWasUpdated = true;
	CurrentTime.Hour++;
	if (CurrentTime.Hour > 23)
	{
		CurrentTime.Hour = 0;
		AdvanceDay();
	}
	pMessanger->UpdateHour(CurrentTime.Hour);
	if (bHasWeatherData && --RemainingWeatherDuration <= 0.f)
	{
		SelectNextWeatherState();
	}
}

void UEnvironmentManager::AdvanceDay()
{
	bTimeWasUpdated = true;
	CurrentTime.DayOfMonth++;
	CurrentTime.DayOfYear++;
	switch (CurrentTime.Month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		if (CurrentTime.DayOfMonth > 31)
		{
			CurrentTime.DayOfMonth = 1;
			AdvanceMonth();
		}
		break;
	case 4:
	case 6:
	case 9:
	case 11:
		if (CurrentTime.DayOfMonth > 30)
		{
			CurrentTime.DayOfMonth = 1;
			AdvanceMonth();
		}
		break;
	case 2:
		// Check for leap year
		if ((CurrentTime.Year % 4 == 0) && (!(CurrentTime.Year % 100 == 0) || (CurrentTime.Year % 400 == 0)))
		{
			if (CurrentTime.DayOfMonth > 29)
			{
				CurrentTime.DayOfMonth = 1;
				AdvanceMonth();
			}
			break;
		}
		else if (CurrentTime.DayOfMonth > 28)
		{
			CurrentTime.DayOfMonth = 1;
			AdvanceMonth();
		}
		break;
	default:
		break;
	}
	pMessanger->UpdateDayOfMonth(CurrentTime.DayOfMonth);
	pMessanger->UpdateDayOfYear(CurrentTime.DayOfYear);
}

void UEnvironmentManager::AdvanceMonth()
{
	bTimeWasUpdated = true;
	CurrentTime.Month++;
	if (CurrentTime.Month > 12)
	{
		CurrentTime.Month = 1;
		AdvanceYear();
	}
	pMessanger->UpdateMonth(CurrentTime.Month);
}

void UEnvironmentManager::AdvanceYear()
{
	bTimeWasUpdated = true;
	CurrentTime.Year++;
	CurrentTime.DayOfYear = 1;
	pMessanger->UpdateYear(CurrentTime.Year);
	pMessanger->UpdateDayOfYear(CurrentTime.DayOfYear);
}

void UEnvironmentManager::SetDayOfYear()
{
	CurrentTime.DayOfYear = 0;
	switch (CurrentTime.Month)
	{
	case 12:
		CurrentTime.DayOfYear += 30;
		[[fallthrough]];
	case 11:
		CurrentTime.DayOfYear += 31;
		[[fallthrough]];
	case 10:
		CurrentTime.DayOfYear += 30;
		[[fallthrough]];
	case 9:
		CurrentTime.DayOfYear += 31;
		[[fallthrough]];
	case 8:
		CurrentTime.DayOfYear += 31;
		[[fallthrough]];
	case 7:
		CurrentTime.DayOfYear += 30;
		[[fallthrough]];
	case 6:
		CurrentTime.DayOfYear += 31;
		[[fallthrough]];
	case 5:
		CurrentTime.DayOfYear += 30;
		[[fallthrough]];
	case 4:
		CurrentTime.DayOfYear += 31;
		[[fallthrough]];
	case 3:
		CurrentTime.DayOfYear += ((CurrentTime.Year % 4 == 0) && (!(CurrentTime.Year % 100 == 0) || (CurrentTime.Year % 400 == 0))) ? 29 : 28;
		[[fallthrough]];
	case 2:
		CurrentTime.DayOfYear += 31;
		[[fallthrough]];
	case 1:
		CurrentTime.DayOfYear += CurrentTime.DayOfMonth;
	}
}

void UEnvironmentManager::CalculateDayLength()
{
	MinuteLength = (DayLengthInMinutes * 60) / 1440;
	TimeDecay = MinuteLength;
}

void UEnvironmentManager::UpdateLighting()
{
}

void UEnvironmentManager::UpdateLightRotation()
{
}

void UEnvironmentManager::UpdateTemperature(float DeltaTime)
{
	TemperatureTickDecay -= DeltaTime;
	if (TemperatureTickDecay > 0.f)
	{
		return;
	}
	TemperatureTickDecay += TemperatureTickFrequency;
	// Temperature logic
	if (IsValid(DailyTemperatureRange) && IsValid(AnnualTemperatureRange))
	{
		CurrentTemperature = DailyTemperatureRange->GetFloatValue(CurrentTime.GetTimeOfDay());
		CurrentTemperature += AnnualTemperatureRange->GetFloatValue(CurrentTime.DayOfYear);
	}
	else if (IsValid(DailyTemperatureRange))
	{
		CurrentTemperature = DailyTemperatureRange->GetFloatValue(CurrentTime.GetTimeOfDay());
	}
	else if (IsValid(AnnualTemperatureRange))
	{
		CurrentTemperature = AnnualTemperatureRange->GetFloatValue(CurrentTime.DayOfYear);
	}
	else
	{
		LOG_WARNING(LogEnvironment, "No valid temperature curve found!");
	}
	if (bUseCelsius)
	{
		CurrentTemperature = ConvertToCelsius(CurrentTemperature);
	}
	CurrentTemperature += CurrentWeatherState.TemeratureModifier;
	if (pMessanger)
	{
		pMessanger->UpdateTemperature(CurrentTemperature);
	}
}

void UEnvironmentManager::SelectNextWeatherState()
{
	if (!IsValid(pWeatherData))
	{
		LOG_WARNING(LogEnvironment, "Weather data is invalid");
		return;
	}
	const FWeatherTransitionList* pTransitions = pWeatherData->TransitionTable.Find(CurrentWeatherType);
	if (!pTransitions || pTransitions->Transitions.IsEmpty())
	{
		LOG_WARNING(LogEnvironment, "No Transition Data available");
		return;
	}
	ESeason CurrentSeason = GetCurrentSeason();
	TArray<const FWeatherTransition*> ValidTransition;
	for (const FWeatherTransition& T : pTransitions->Transitions)
	{
		if (!T.bSeasonRestricted || T.AllowedSeasons.Contains(CurrentSeason))
		{
			ValidTransition.Add(&T);
		}
	}
	if (ValidTransition.IsEmpty())
	{
		LOG_ERROR(LogEnvironment, "No valid transitions available");
		return;
	}
	float TotalWeight = 0.f;
	for (const FWeatherTransition* T : ValidTransition)
	{
		TotalWeight += T->Weight;
	}
	float Roll = FMath::FRandRange(0.f, TotalWeight);
	float Accumulated = 0.f;
	const FWeatherTransition* Selected = ValidTransition.Last();
	for (const FWeatherTransition* T : ValidTransition)
	{
		Accumulated += T->Weight;
		if (Roll <= Accumulated)
		{
			Selected = T;
			break;
		}
	}
	EWeatherType NewWeatherType = Selected->ToWeatherType;
	if (const FWeatherState* pDefaults = pWeatherData->WeatherStateDefaults.Find(NewWeatherType))
	{
		CurrentWeatherState = *pDefaults;
		CurrentWeatherType = NewWeatherType;
		RemainingWeatherDuration = FMath::FRandRange(CurrentWeatherState.MinDuration, CurrentWeatherState.MaxDuration);
		if (pMessanger)
		{
			pMessanger->UpdateWeather();
		}
	}
	else
	{
		LOG_ERROR(LogEnvironment, "No Weather Type found");
	}
}

ESeason UEnvironmentManager::GetCurrentSeason() const
{
	if (CurrentTime.DayOfYear < VERNALEQUINOX || CurrentTime.DayOfYear >= WINTERSOLSTICE)
	{
		return ESeason::Winter;
	}
	if (CurrentTime.DayOfYear < SUMMERSOLSTICE)
	{
		return ESeason::Spring;
	}
	if (CurrentTime.DayOfYear < AUTUMNEQUINOX)
	{
		return ESeason::Summer;
	}
	return ESeason::Autumn;
}

float UEnvironmentManager::ConvertToCelsius(const float Fahrenheit)
{
	return (5.f / 9.f) * (Fahrenheit - 32);
}