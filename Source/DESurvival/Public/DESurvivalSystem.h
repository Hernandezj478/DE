// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DESurvivalSystem.generated.h"

/**
 * 
 */
class UDEStaminaSystem;

UCLASS()
class DESURVIVAL_API UDESurvivalSystem : public UObject
{
	GENERATED_BODY()
public:
	void Initialize();
	void Deinitialize();
	
	// Accessors (used by character / gameplay systems)
	UPROPERTY()
	UDEStaminaSystem* GetStaminaSystem() const;
private:
	// Stamina system
	UDEStaminaSystem* StaminaSystem;
	// Health system
	
	// Satiation system
	
	// Hydration system
};
