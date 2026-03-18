// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "FTimeData.h"
#include "MessagingSubsystem.generated.h"

#pragma region TimeManager
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTimeChangedDelegate, FTimeData, TimeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMinuteChangedDelegate, int, Minute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHourChangedDelegate, int, Hour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDayOfYearChangedDelegate, int, DayOfYear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMonthChangedDelegate, int, Month);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYearChangedDelegate, int, Year);
#pragma endregion

#pragma region Temperature
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTemperatureChangedDelegate, float, CurrentTemp);
#pragma endregion

#pragma region Stats
DECLARE_MULTICAST_DELEGATE_OneParam(FExhaustionChangedDelegate, bool);
#pragma endregion

UCLASS()
class MESSAGEHANDLER_API UMessagingSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
#pragma region TimeDelegates
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FTimeChangedDelegate OnTimeChanged;
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FMinuteChangedDelegate OnMinuteChanged;
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FHourChangedDelegate OnHourChanged;
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FDayOfYearChangedDelegate OnDayChanged;
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FMonthChangedDelegate OnMonthChanged;
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FYearChangedDelegate OnYearChanged;
#pragma endregion
	
#pragma region TemperatureDelegate
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FTemperatureChangedDelegate OnTemperatureChanged;
#pragma endregion

#pragma region StatDelegates
	//UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FExhaustionChangedDelegate OnExhaustionChanged;
#pragma endregion

#pragma region TriggerFunctions
	void UpdateTime(FTimeData NewTimeData);
	void UpdateMinute(int Minute);
	void UpdateHourOfDay(int NewHour);
	void UpdateDayOfYear(int NewDayOfYear);
	void UpdateMonth(int NewMonth);
	void UpdateYear(int NewYear);
	
	void UpdateTemperature(int NewTemperature);
	
#pragma endregion

	static UMessagingSubsystem* Get();
};
