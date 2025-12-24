// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DEStatComponent.h"

#include "DECore/Public/DELogger.h"


// Sets default values for this component's properties
UDEStatComponent::UDEStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UDEStatComponent::GetStamina() const
{
	return CurrentStamina;
}

void UDEStatComponent::ConsumeStamina(float Amount)
{
	CurrentStamina -= Amount;
	ClampStamina();
	
	UE_LOG(LogDECharacters, Display, TEXT("Stamina consumed: %.1f | Current: %.1f"), Amount, CurrentStamina);
}

void UDEStatComponent::RestoreStamina(float Amount)
{
	CurrentStamina += Amount;
	
	ClampStamina();
}


// Called when the game starts
void UDEStatComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentStamina = MaxStamina;
	
	UE_LOG(LogDECharacters, Display, TEXT("StatComponent initialized. Stamina = %.1f"), CurrentStamina);
}

void UDEStatComponent::ClampStamina()
{
	CurrentStamina = FMath::Clamp(CurrentStamina, 0.f, MaxStamina);
}


