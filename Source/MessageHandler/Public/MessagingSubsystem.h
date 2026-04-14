// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "MessagingSubsystem.generated.h"

#pragma region TimeManager
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FTimeChangedDelegate, 
	int, DayOfYear, int, Year, int, Month, int, DayOfMonth, int, Hour, int, Minute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDayOfYearChangedDelegate, int, DayOfYear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYearChangedDelegate, int, Year);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMonthChangedDelegate, int, Month);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDayOfMonthChangedDelegate, int, DayOfMonth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHourChangedDelegate, int, Hour);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMinuteChangedDelegate, int, Minute);
#pragma endregion

#pragma region Temperature
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTemperatureChangedDelegate, float, CurrentTemp);
#pragma endregion

#pragma region Stats
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FExhaustionChangedDelegate, bool, NewExhaustion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStarvationChangedDelegate, bool, NewStarvation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDehydrationChangedDelegate, bool, NewDehydration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLowBloodChangedDelegate, bool, NewLowBlood);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthChangedDelegate, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStaminaChangedDelegate, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSatiationChangedDelegate, float, NewSatiation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHydrationChangedDelegate, float, NewHydration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBloodChangedDelegate, float, NewBlood);
#pragma endregion

#pragma region Movement
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSprintChangedDelegate, bool, NewSprint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCrouchChangedDelegate, bool, NewCrouch);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJumpChangedDelegate, bool, NewJump);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFallChangedDelegate, bool, NewFall);
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
	FDayOfYearChangedDelegate OnDayOfYearChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FYearChangedDelegate OnYearChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FMonthChangedDelegate OnMonthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FDayOfMonthChangedDelegate OnDayOfMonthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FHourChangedDelegate OnHourChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FMinuteChangedDelegate OnMinuteChanged;
#pragma endregion
	
#pragma region TemperatureDelegate
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Time")
	FTemperatureChangedDelegate OnTemperatureChanged;
#pragma endregion

#pragma region StatDelegates
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FExhaustionChangedDelegate OnExhaustionChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FStarvationChangedDelegate OnStarvationChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FDehydrationChangedDelegate OnDehydrationChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FLowBloodChangedDelegate OnLowBloodChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FHealthChangedDelegate OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FStaminaChangedDelegate OnStaminaChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FSatiationChangedDelegate OnSatiationChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FHydrationChangedDelegate OnHydrationChanged;
	UPROPERTY(BlueprintAssignable, Category = "Boradcast Message|Stat")
	FBloodChangedDelegate OnBloodChanged;

#pragma endregion

#pragma region Movement
	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Movement")
	FSprintChangedDelegate OnSprintChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Movement")
	FCrouchChangedDelegate OnCrouchChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Movement")
	FJumpChangedDelegate OnJumpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Broadcast Message|Movement")
	FFallChangedDelegate OnFallingChanged;
#pragma endregion

#pragma region TriggerFunctions
	void UpdateTime(int NewDayOfYear, int NewYear, int NewMonth, int NewDayOfMonth, int NewHour, int NewMinute);
	void UpdateDayOfYear(int NewDayOfYear);
	void UpdateYear(int NewYear);
	void UpdateMonth(int NewMonth);
	void UpdateDayOfMonth(int NewDayOfMonth);
	void UpdateHour(int NewHour);
	void UpdateMinute(int Minute);
	
	void UpdateTemperature(int NewTemperature);
	
	void UpdateExhustion(bool NewExhaustion);
	void UpdateHealth(float NewHealth);
	void UpdateStamina(float NewStamina);
	void UpdateSatiation(float NewSatiation);
	void UpdateHydration(float NewHydration);
	void UpdateBlood(float NewBlood);


	void UpdateSprint(bool NewSprint);
	void UpdateCrouch(bool NewCrouch);
	void UpdateJump(bool NewJump);
	void UpdateFalling(bool NewFall);
#pragma endregion

	static UMessagingSubsystem* Get();
};
