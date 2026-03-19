// Fill out your copyright notice in the Description page of Project Settings.


#include "LightController.h"

#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Curves/CurveLinearColor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Logger.h"
#include "MessagingSubsystem.h"

// Sets default values
ALightController::ALightController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ALightController::TimeChangedUpdate(int DayOfYear, int Year, int Month, 
	int DayOfMonth, int Hour, int Minute)
{
	CurrentTime.DayOfYear = DayOfYear;
	CurrentTime.Year = Year;
	CurrentTime.Month = Month;
	CurrentTime.DayOfMonth = DayOfMonth;
	CurrentTime.Hour = Hour;
	CurrentTime.Minute = Minute;
	CurrentTimeOfDay = CurrentTime.GetTimeOfDay();
	UpdateFromNewTimeData();
}

// Called when the game starts or when spawned
void ALightController::BeginPlay()
{
	Super::BeginPlay();
	if (bHasDayNightCycle)
	{
		if (UMessagingSubsystem* Messaging = UMessagingSubsystem::Get())
		{
			Messaging->OnTimeChanged.AddDynamic(this, &ALightController::TimeChangedUpdate);
		}
	}
}

void ALightController::UpdateFromNewTimeData()
{
	UpdateSunLight();
	UpdateMoonLight();
	UpdateSkyLight();
}

void ALightController::UpdateSunLight()
{
	if (!IsValid(SunLightActor) || !IsValid(DailySunRotation))
	{
		Logger::GetInstance()->AddMessage("ALightController::UpdateSunLight: SunLightActor or DailySunRotation is not valid", ERROR);
		return;
	}
	float NewLightIntensity = DailySunRotation->GetUnadjustedLinearColorValue(CurrentTimeOfDay).A;
	FLinearColor ColorAsRotation = DailySunRotation->GetUnadjustedLinearColorValue(CurrentTimeOfDay);
	if (IsValid(AnnualSunRotation))
	{
		NewLightIntensity += AnnualSunRotation->GetUnadjustedLinearColorValue(CurrentTimeOfDay).A;
		ColorAsRotation += AnnualSunRotation->GetUnadjustedLinearColorValue(CurrentTimeOfDay);
	}
	
	FRotator NewLightRotation = FRotator(ColorAsRotation.G, ColorAsRotation.B, ColorAsRotation.R);
	SunLightActor->SetActorRotation(NewLightRotation);
	NewLightIntensity = FMath::Clamp(NewLightIntensity, 0.0f, MaxSunIntensity);
	SunLightActor->GetLightComponent()->Intensity = NewLightIntensity;
	SunLightActor->GetLightComponent()->UpdateColorAndBrightness();
}

void ALightController::UpdateSkyLight()
{
	if (!IsValid(SkyLight))
	{
		return;
	}
	float NewLightIntensity = SkyLightDailyColor->GetUnadjustedLinearColorValue(CurrentTimeOfDay).A;
	SkyLight->GetLightComponent()->SetIntensity(NewLightIntensity);
	FLinearColor NewSkyLightColor = SkyLightDailyColor->GetUnadjustedLinearColorValue(CurrentTimeOfDay);
	SkyLight->GetLightComponent()->SetLightColor(NewSkyLightColor);
}

void ALightController::UpdateMoonLight()
{
	
}

