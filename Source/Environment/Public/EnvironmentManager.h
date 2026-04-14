// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FTimeData.h"
#include "EnvironmentManager.generated.h"

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
	
private:
	bool bCanEverTick = true;
	bool bLogTick = true;
	bool bUseDayNightCycle = true;
	
	FTimeData CurrentTime;
	float DayLengthInMinutes = 5;
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

	const float TemperatureTickFrequency = 1.f;
	float TemperatureTickDecay = 0.f;
	bool bHasTemperatureData = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Environment|Temperature")
	float CurrentTemperature = 70.f;

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

	float ConvertToCelsius(const float Fahrenheit);
};
