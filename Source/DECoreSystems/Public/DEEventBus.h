// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DEEventBus.generated.h"

/**
 * 
 */
UCLASS()
class DECORESYSTEMS_API UDEEventBus : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	
	
	// Global accessor
	static UDEEventBus* Get();
};
