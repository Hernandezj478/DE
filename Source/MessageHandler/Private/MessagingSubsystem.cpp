// Fill out your copyright notice in the Description page of Project Settings.


#include "MessagingSubsystem.h"
#include "Logger.h"

void UMessagingSubsystem::UpdateTime(int NewDayOfYear, int NewYear, int NewMonth, 
	int NewDayOfMonth, int NewHour, int NewMinute)
{
	OnTimeChanged.Broadcast(NewDayOfYear, NewYear, NewMonth, NewDayOfMonth, NewHour, NewMinute);
}

void UMessagingSubsystem::UpdateDayOfYear(int NewDayOfYear)
{
	OnDayOfYearChanged.Broadcast(NewDayOfYear);
}

void UMessagingSubsystem::UpdateYear(int NewYear)
{
	OnYearChanged.Broadcast(NewYear);
}

void UMessagingSubsystem::UpdateMonth(int NewMonth)
{
	OnMonthChanged.Broadcast(NewMonth);
}

void UMessagingSubsystem::UpdateDayOfMonth(int NewDayOfMonth)
{
	OnDayOfMonthChanged.Broadcast(NewDayOfMonth);
}

void UMessagingSubsystem::UpdateHour(int NewHour)
{
	OnHourChanged.Broadcast(NewHour);
}

void UMessagingSubsystem::UpdateMinute(int Minute)
{
	OnMinuteChanged.Broadcast(Minute);
}

void UMessagingSubsystem::UpdateTemperature(int NewTemperature)
{
	OnTemperatureChanged.Broadcast(NewTemperature);
}

void UMessagingSubsystem::UpdateExhustion(bool NewExhaustion)
{
	OnExhaustionChanged.Broadcast(NewExhaustion);
}

UMessagingSubsystem* UMessagingSubsystem::Get()
{
	if (!GEngine)
	{
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UMessagingSubsystem>();
}
