// Fill out your copyright notice in the Description page of Project Settings.


#include "MessagingSubsystem.h"
#include "Logger.h"

void UMessagingSubsystem::UpdateTime(FTimeData NewTimeData)
{
	OnTimeChanged.Broadcast(NewTimeData);
}

void UMessagingSubsystem::UpdateMinute(int Minute)
{
	OnMinuteChanged.Broadcast(Minute);
}

void UMessagingSubsystem::UpdateHourOfDay(int NewHour)
{
	OnHourChanged.Broadcast(NewHour);
}

void UMessagingSubsystem::UpdateDayOfYear(int NewDayOfYear)
{
	OnDayChanged.Broadcast(NewDayOfYear);
}

void UMessagingSubsystem::UpdateMonth(int NewMonth)
{
	OnMonthChanged.Broadcast(NewMonth);
}

void UMessagingSubsystem::UpdateYear(int NewYear)
{
	OnYearChanged.Broadcast(NewYear);
}

void UMessagingSubsystem::UpdateTemperature(int NewTemperature)
{
	OnTemperatureChanged.Broadcast(NewTemperature);
}

UMessagingSubsystem* UMessagingSubsystem::Get()
{
	if (!GEngine)
	{
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UMessagingSubsystem>();
}
