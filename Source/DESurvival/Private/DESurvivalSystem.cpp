// Fill out your copyright notice in the Description page of Project Settings.


#include "DESurvivalSystem.h"

#include "DELogger.h"
#include "DEStaminaSystem.h"

void UDESurvivalSystem::Initialize()
{
	StaminaSystem = NewObject<UDEStaminaSystem>(this);
	if (!StaminaSystem)
	{
		UE_LOG(LogDESurvival, Error, TEXT("Failed to create StaminaSystem"));
		return;
	}
	StaminaSystem->Initialize();
}

void UDESurvivalSystem::Deinitialize()
{
	if (StaminaSystem)
	{
		StaminaSystem->Deinitialize();
		StaminaSystem = nullptr;
	}
}

UDEStaminaSystem* UDESurvivalSystem::GetStaminaSystem() const
{
	return StaminaSystem;
}
