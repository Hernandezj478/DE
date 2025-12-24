// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DEStatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DECHARACTERS_API UDEStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDEStatComponent();
	
	// Getters
	UFUNCTION(BlueprintCallable)
	float GetStamina() const;
	
	// Modifiers
	void ConsumeStamina(float Amount);
	void RestoreStamina(float Amount);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	float CurrentStamina;
	
	// Max values (temp constants)
	float MaxStamina = 100.0f;
	
	void ClampStamina();
};
