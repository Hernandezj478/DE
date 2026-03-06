// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"

#include <format>
#include <string>

#include "MessagingSubsystem.h"
#include "Logger.h"

void UEnvironmentManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogEnvironment, Log, TEXT("UEnvironmentManager::Initialize"));
	Logger::GetInstance()->AddMessage("UEnvironmentManager::Initialize", DEBUG);
	if (UWorld* pWorld = GetWorld())
	{
		FString MapName = pWorld->GetMapName();
		MapName = pWorld->RemovePIEPrefix(MapName);
		if (MapName.Equals("MainMenu", ESearchCase::IgnoreCase))
		{
			UE_LOG(LogEnvironment, Log, TEXT("UEnvironmentManager::Initialize: MainMenu"));
			Logger::GetInstance()->AddMessage("UEnvironmentManager::Initialize: MainMenu", DEBUG);
			bCanEverTick = false;
		}
		SetTickableTickType(ETickableTickType::Conditional);
	}

}

void UEnvironmentManager::Deinitialize()
{
	Super::Deinitialize();
}

void UEnvironmentManager::Tick(float DeltaTime)
{
	UpdateTime(DeltaTime);
	UpdateTimeOfDayRef();
	if (bTimeWasUpdated && pMessageManager)
	{
		pMessageManager->UpdateTime(CurrentTime);
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
	UE_LOG(LogEnvironment, Log, TEXT("UEnvironment::OnWorldBeginPlay"));
	Logger::GetInstance()->AddMessage(FString::Printf(TEXT("UEnvironmentManager::OnWorldBeginPlay: Day length calculated %f minutes"), MinuteLength), DEBUG);
	
	if (UMessagingSubsystem* MessagingSubsystem = UMessagingSubsystem::Get())
	{
		pMessageManager = MessagingSubsystem;
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
	if (pMessageManager)
	{
		pMessageManager->UpdateMinute(CurrentTime.Minute);
	}
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
	if (pMessageManager)
	{
		pMessageManager->UpdateHourOfDay(CurrentTime.Hour);
	}
	Logger::GetInstance()->AddMessage("Time has advanced 1 hour", DEBUG);
}

void UEnvironmentManager::AdvanceDay()
{
	bTimeWasUpdated = true;
	AddDayOfYear();
	CurrentTime.DayOfMonth++;
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
		if (CurrentTime.DayOfMonth > 28)
		{
			CurrentTime.DayOfMonth = 1;
			AdvanceMonth();
		}
		break;
	default:
		break;
	}
	if (pMessageManager)
	{
		pMessageManager->UpdateDayOfYear(CurrentTime.DayOfYear);
	}
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
	if (pMessageManager)
	{
		pMessageManager->UpdateMonth(CurrentTime.Month);
	}
}

void UEnvironmentManager::AdvanceYear()
{
	bTimeWasUpdated = true;
	CurrentTime.Year++;
	if (pMessageManager)
	{
		pMessageManager->UpdateYear(CurrentTime.Year);
	}
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

void UEnvironmentManager::UpdateTimeOfDayRef()
{
	CurrentTimeOfDay = (CurrentTime.Hour * 60) + CurrentTime.Minute;
}

void UEnvironmentManager::UpdateLighting()
{
}

void UEnvironmentManager::UpdateLightRotation()
{
}

void UEnvironmentManager::AddDayOfYear()
{
	CurrentTime.DayOfYear++;
	if (CurrentTime.DayOfYear > ((CurrentTime.Year % 4 == 0) && (!(CurrentTime.Year % 100 == 0) || (CurrentTime.Year % 400 == 0)) ? 366 : 365))
	{
		CurrentTime.DayOfYear = 1;
	}
}
