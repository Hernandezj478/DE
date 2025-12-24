// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DEEventBus.generated.h"

/**
 * 
 */
UCLASS()
class DECORE_API UDEEventBus : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	// Example placeholder for delegates (remove later)
	DECLARE_MULTICAST_DELEGATE(ESimpleEvent);
	DECLARE_MULTICAST_DELEGATE_OneParam(FIntEvent, int32);
	
	// Delete these later, only exist to validate pattern
	ESimpleEvent OnTestEvent;
	FIntEvent OnTestIntEvent;
	
	// Convenience accessor
	static UDEEventBus* Get();
};
