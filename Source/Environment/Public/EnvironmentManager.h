// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FTimeData.h"
#include "WeatherType.h"
#include "WeatherState.h"
#include "Season.h"
#include "EnvironmentManager.generated.h"

#define VERNALEQUINOX 80
#define AUTUMNEQUINOX 264
#define SUMMERSOLSTICE 172
#define WINTERSOLSTICE 355

UCLASS()
class ENVIRONMENT_API UEnvironmentManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual TStatId GetStatId() const override;
	
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	EWeatherType GetCurrentWeatherType() const { return CurrentWeatherType; }
	FWeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

private:
	bool bCanEverTick = true;
	bool bLogTick = true;
	bool bUseDayNightCycle = true;
	
	FTimeData CurrentTime;
	float DayLengthInMinutes = 60;
	float TimeDecay = 0;
	float MinuteLength = 0;
	
	bool bTimeWasUpdated = true;
	int CurrentTimeOfDay = 0;
	
	bool bUseCelsius = true;

	UPROPERTY()
	class UMessagingSubsystem* pMessanger;
	UPROPERTY()
	class ADEWorldSettings* pWorldSettings;
	UPROPERTY()
	class UCurveFloat* DailyTemperatureRange;
	UPROPERTY()
	class UCurveFloat* AnnualTemperatureRange;
	UPROPERTY()
	class UWeatherTransitionData* pWeatherData;

	const float TemperatureTickFrequency = 1.f;
	float TemperatureTickDecay = 0.f;
	bool bHasTemperatureData = false;
	bool bHasWeatherData = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Environment|Temperature")
	float CurrentTemperature = 70.f;

	UPROPERTY()
	EWeatherType CurrentWeatherType = EWeatherType::Clear;
	UPROPERTY()
	FWeatherState CurrentWeatherState;
	UPROPERTY()
	float RemainingWeatherDuration = 0.0f;

	void UpdateTime(float DeltaTime);
	void AdvanceMinute();
	void AdvanceHour();
	void AdvanceDay();
	void AdvanceMonth();
	void AdvanceYear();
	void SetDayOfYear();
	void CalculateDayLength();
	void UpdateLighting();
	void UpdateLightRotation();
	void UpdateTemperature(float DeltaTime);
	void SelectNextWeatherState();
	ESeason GetCurrentSeason() const;

	float ConvertToCelsius(const float Fahrenheit);
};
