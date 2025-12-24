// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DESurvivalSystem.generated.h"

class UDEStaminaSystem;

UCLASS()
class DESURVIVAL_API UDESurvivalSystem : public UObject
{
	GENERATED_BODY()
public:
	void Initialize();
	void Deinitialize();
	
	UDEStaminaSystem* GetStaminaSystem() const;
private:
	
	UPROPERTY()
	UDEStaminaSystem* StaminaSystem;
};
