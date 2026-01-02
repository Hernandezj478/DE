// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DEMovementStateComponent.generated.h"

class UDEEventBus;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DECHARACTERS_API UDEMovementStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDEMovementStateComponent();

	bool CanSprint() const;
	void RequestSprint(const bool& bRequested);
	void RequestCrouch(const bool& bRequested);
	float GetWalkSpeed() const;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	
	class UCharacterMovementComponent* MovementComponent; 
	
	bool bSprintRequested = false;
	bool bCrouchRequested = false;
	bool bIsSprinting = false;
	bool bIsFalling = false;
	bool bIsCrouching = false;
	bool bCanSprint = true;
	
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	float WalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	float SprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	float CrouchSpeed = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	float ExhaustedSpeed = 80.f;
	
	void EvaluateSprintState();
	void EvaluateCrouchState();
	// Event handlers
	void HandleExhaustionStart(AActor* Actor);
	void HandleExhaustionEnd(AActor* Actor);
	
	UFUNCTION()
	void OnMovementModeChanged(ACharacter* Character, EMovementMode NewMovementMode, uint8 PreviousCustomMode);
	
	void SprintStart();
	void SprintEnd();
	void CrouchStart();
	void CrouchEnd();
	void FallingStart();
	void FallingEnd();
	
	void ApplyWalkSpeed();
	void ApplySprintSpeed();
	void ApplyCrouchSpeed();
	void ApplyExhaustedSpeed();
	
};
