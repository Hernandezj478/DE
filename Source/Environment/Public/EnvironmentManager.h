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
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(AllowPrivateAccess=true))
	float DayLengthInMinutes = 5;
	float TimeDecay = 0;
	float MinuteLength = 0;
	
	bool bTimeWasUpdated = true;
	int CurrentTimeOfDay = 0;
	
	class UMessagingSubsystem* pMessanger;

	void UpdateTime(float DeltaTime);
	void AdvanceMinute();
	void AdvanceHour();
	void AdvanceDay();
	void AdvanceMonth();
	void AdvanceYear();
	void SetDayOfYear();
	UFUNCTION(BlueprintCallable,meta=(AllowPrivateAccess=true))
	void CalculateDayLength();
	void UpdateLighting();
	void UpdateLightRotation();
};
