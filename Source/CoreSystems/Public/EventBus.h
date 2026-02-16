// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "EventBus.generated.h"

/**
 * 
 */

#pragma region Movement

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSprintStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSprintEnd, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCrouchStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCrouchEnd, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExhaustionStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnExhaustionEnd, AActor*);

#pragma endregion

#pragma region Stats

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStarvationStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStarvationEnd, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDehydrationStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDehydrationEnd, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLowBloodStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLowBloodEnd, AActor*);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFallingStart, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFallingEnd, AActor*);

#pragma endregion
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*);



UCLASS()
class CORESYSTEMS_API UEventBus : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	//------------------------
	// Delegates
	//------------------------
#pragma region Movement
	
	FOnExhaustionStart OnExhaustionStart;
	FOnExhaustionEnd OnExhaustionEnd;
	FOnSprintStart OnSprintStart;
	FOnSprintEnd OnSprintEnd;
	FOnCrouchStart OnCrouchStart;
	FOnCrouchEnd OnCrouchEnd;
	
	FOnFallingStart OnFallingStart;
	FOnFallingEnd OnFallingEnd;
	
#pragma endregion
	
	FOnStarvationStart OnStarvationStart;
	FOnStarvationEnd OnStarvationEnd;
	FOnDehydrationStart OnDehydrationStart;
	FOnDehydrationEnd OnDehydrationEnd;
	FOnLowBloodStart OnLowBloodStart;
	FOnLowBloodEnd OnLowBloodEnd;
	FOnDeath FOnDeath;
	
	// Global accessor
	static UEventBus* Get();
};
