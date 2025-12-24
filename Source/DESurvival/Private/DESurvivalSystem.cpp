// Fill out your copyright notice in the Description page of Project Settings.


#include "DESurvivalSystem.h"
#include "DEStaminaSystem.h"

#include "DECore/Public/DELogger.h"

void UDESurvivalSystem::Initialize()
{
	// Subsystems are owned by the survival system
	StaminaSystem = NewObject<UDEStaminaSystem>(this);
	if (!StaminaSystem)
	{
		UE_LOG(LogDESurvival, Warning, TEXT("Stamina system could not be created"));
	}
	if (StaminaSystem)
	{
		StaminaSystem->Initialize();
	}
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
