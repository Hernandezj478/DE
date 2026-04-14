// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MovementStateComponent.generated.h"

class UMessagingSubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHARACTERMOVEMENT_API UMovementStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMovementStateComponent();

	bool CanSprint() const;
	bool RequestSprint(const bool& bRequested);
	bool RequestCrouch(const bool& bRequested);
	float GetWalkSpeed() const;
	
	FORCEINLINE bool GetSprintState() const { return bIsSprinting; }
	FORCEINLINE bool GetCrouchState() const { return bIsCrouching; }
	FORCEINLINE bool GetFallState() const { return bIsFalling; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	class UCharacterMovementComponent* MovementComponent;
	UMessagingSubsystem* pMessanger;

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

	UFUNCTION()
	void OnExhaustionChanged(bool IsExhausted);

	UFUNCTION()
	void OnMovementModeChanged(ACharacter* Character, EMovementMode NewMovementMode, uint8 PreviousCustomMode);
	
	void UpdateSprintState(bool bSprint);
	void UpdateCrouchState(bool bCrouch);

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
