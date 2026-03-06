// Fill out your copyright notice in the Description page of Project Settings.


#include "LightController.h"
#include "MessagingSubsystem.h"
#include "Components/LightComponent.h"
#include "Curves/CurveLinearColor.h"
#include "Engine/DirectionalLight.h"
#include "Logger.h"

// Sets default values
ALightController::ALightController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ALightController::TimeChangedUpdate(FTimeData TimeData)
{
	Logger::GetInstance()->AddMessage((TEXT("TimeChangedUpdate: %s"), *TimeData.GetTimeString()), DEBUG);
	CurrentTime = TimeData;
	UpdateFromNewTimeData();
}

// Called when the game starts or when spawned
void ALightController::BeginPlay()
{
	Super::BeginPlay();
	if (bHasDayNightCycle)
	{
		// GetWorld()->GetSubsystem<UMessagingSubsystem>()->OnTimeChanged.AddDynamic(this, &ALightController::TimeChangedUpdate);
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
		return;
	}
	float CurrentTimeOfDay = CurrentTime.GetTimeOfDay();
	float NewLightIntensity = DailySunRotation->GetUnadjustedLinearColorValue(CurrentTimeOfDay).A;
	FLinearColor ColorAsRotation = DailySunRotation->GetLinearColorValue(CurrentTimeOfDay);
	
	if (IsValid(AnnualSunRotation))
	{
		NewLightIntensity += AnnualSunRotation->GetUnadjustedLinearColorValue(CurrentTimeOfDay).A;
		ColorAsRotation += AnnualSunRotation->GetLinearColorValue(CurrentTimeOfDay);
	}
	
	FRotator NewLightRotation = FRotator(ColorAsRotation.G, ColorAsRotation.B, ColorAsRotation.R);
	SunLightActor->SetActorRotation(NewLightRotation);
	NewLightIntensity = FMath::Clamp(NewLightIntensity, 0.0f, MaxSunIntensity);
	SunLightActor->GetLightComponent()->Intensity = NewLightIntensity;
	SunLightActor->GetLightComponent()->UpdateColorAndBrightness();
}

void ALightController::UpdateSkyLight()
{
	
}

void ALightController::UpdateMoonLight()
{
	
}

