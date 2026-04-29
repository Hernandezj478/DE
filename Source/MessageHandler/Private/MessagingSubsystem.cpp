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

void UMessagingSubsystem::UpdateHealth(float NewHealth)
{
	OnHealthChanged.Broadcast(NewHealth);
}

void UMessagingSubsystem::UpdateStamina(float NewStamina)
{
	OnStaminaChanged.Broadcast(NewStamina);
}

void UMessagingSubsystem::UpdateSatiation(float NewSatiation)
{
	OnSatiationChanged.Broadcast(NewSatiation);
}

void UMessagingSubsystem::UpdateHydration(float NewHydration)
{
	OnHydrationChanged.Broadcast(NewHydration);
}

void UMessagingSubsystem::UpdateBlood(float NewBlood)
{
	OnBloodChanged.Broadcast(NewBlood);
}

void UMessagingSubsystem::UpdateSprint(bool NewSprint)
{
	OnSprintChanged.Broadcast(NewSprint);
}

void UMessagingSubsystem::UpdateCrouch(bool NewCrouch)
{
	OnCrouchChanged.Broadcast(NewCrouch);
}

void UMessagingSubsystem::UpdateJump(bool NewJump)
{
	OnJumpChanged.Broadcast(NewJump);
}

void UMessagingSubsystem::UpdateFalling(bool NewFall)
{
	OnFallingChanged.Broadcast(NewFall);
}

void UMessagingSubsystem::UpdateWeather()
{
	OnWeatherChanged.Broadcast();
}

UMessagingSubsystem* UMessagingSubsystem::Get()
{
	if (!GEngine)
	{
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UMessagingSubsystem>();
}
