// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FTimeData.h"
#include "LightController.generated.h"

class UMessagingSubsystem;

UCLASS()
class ENVIRONMENT_API ALightController : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALightController();

	UFUNCTION()
	void TimeChangedUpdate(int DayOfYear, int Year, int Month,
		int DayOfMonth, int Hour, int Minute);
		
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	bool bHasDayNightCycle = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class ADirectionalLight* SunLightActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UCurveLinearColor* DailySunRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UCurveLinearColor* AnnualSunRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class ASkyLight* SkyLight;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UCurveLinearColor* SkyLightDailyColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	float MaxSunIntensity = 10.f;
	
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess = "true"))
	FTimeData CurrentTime;
	
	float CurrentTimeOfDay = 0;
	void UpdateFromNewTimeData();
	void UpdateSunLight();
	void UpdateSkyLight();
	void UpdateMoonLight();
};
