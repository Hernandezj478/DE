// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DEEventBus.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExhaustionStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExhaustionEnd, AActor*);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*);
UCLASS()
class DECORESYSTEMS_API UDEEventBus : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	//------------------------
	// Delegates
	//------------------------
	FOnExhaustionStart OnExhaustionStart;
	FOnExhaustionEnd OnExhaustionEnd;
	
	FOnDeath FOnDeath;
	
	// Global accessor
	static UDEEventBus* Get();
};
