// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"
#include "MessagingSubsystem.h"
#include "DEWorldSettings.h"
#include "Logger.h"

void UEnvironmentManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Logger::GetInstance()->AddMessage("UEnvironmentManager::Initialize", DEBUG);
	if (UWorld* pWorld = GetWorld())
	{
		FString MapName = pWorld->GetMapName();
		MapName = pWorld->RemovePIEPrefix(MapName);
		if (MapName.Equals("MainMenu", ESearchCase::IgnoreCase))
		{
			Logger::GetInstance()->AddMessage("UEnvironmentManager::Initialize: MainMenu", DEBUG);
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
	Super::OnWorldBeginPlay(InWorld);
	CalculateDayLength();
	pWorldSettings = Cast<ADEWorldSettings>(GetWorld()->GetWorldSettings());
	if (pWorldSettings)
	{
		if (!pWorldSettings->DailyTemperatureRange.IsNull())
		{
			DailyTemperatureRange = pWorldSettings->DailyTemperatureRange.LoadSynchronous();
		}
		if (!pWorldSettings->AnnualTemperatureRange.IsNull())
		{
			AnnualTemperatureRange = pWorldSettings->AnnualTemperatureRange.LoadSynchronous();
		}
	}
	if (!IsValid(DailyTemperatureRange) && !IsValid(AnnualTemperatureRange))
	{
		Logger::GetInstance()->AddMessage("UEnvironmentManager::OnWorldBeginPlay - Daily/Annual Temperature Range not valid", DEBUG);
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
	case 11:
		CurrentTime.DayOfYear += 31;
	case 10:
		CurrentTime.DayOfYear += 30;
	case 9:
		CurrentTime.DayOfYear += 31;
	case 8:
		CurrentTime.DayOfYear += 31;
	case 7:
		CurrentTime.DayOfYear += 30;
	case 6:
		CurrentTime.DayOfYear += 31;
	case 5:
		CurrentTime.DayOfYear += 30;
	case 4:
		CurrentTime.DayOfYear += 31;
	case 3:
		CurrentTime.DayOfYear += ((CurrentTime.Year % 4 == 0) && (!(CurrentTime.Year % 100 == 0) || (CurrentTime.Year % 400 == 0))) ? 29 : 28;
	case 2:
		CurrentTime.DayOfYear += 31;
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
		Logger::GetInstance()->AddMessage("UEnvironmentManager::UpdateTemperature - No valid temperature curve found", WARNING);
	}
	if (bUseCelsius)
	{
		CurrentTemperature = ConvertToCelsius(CurrentTemperature);
	}
	pMessanger->UpdateTemperature(CurrentTemperature);
}

float UEnvironmentManager::ConvertToCelsius(const float Fahrenheit)
{
	return (5.f / 9.f) * (Fahrenheit - 32);
}