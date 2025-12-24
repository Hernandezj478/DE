// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DEStaminaSystem.generated.h"

UCLASS()
class DESURVIVAL_API UDEStaminaSystem : public UObject
{
	GENERATED_BODY()
public:
	void Initialize();
	void Deinitialize();
	
	// Hooks for later integration
	void StartSprinting();
	void StopSprinting();
};
