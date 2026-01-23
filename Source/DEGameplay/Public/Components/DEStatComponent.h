// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDEStatType.h"
#include "FDEStat.h"
#include "DEStatComponent.generated.h"

class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEGAMEPLAY_API UDEStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDEStatComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	float GetStatPercentile(const EDEStatType Stat) const;
	
	UFUNCTION(BlueprintCallable)
	bool CanJump();
	
	UFUNCTION(BlueprintCallable)
	void ConsumeJumpStamina();
	
	UFUNCTION(BlueprintCallable)
	void AdjustStat(const EDEStatType Stat, const float& Amount);

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
#pragma region Stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FDEStat Health;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FDEStat Stamina;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FDEStat Blood;	// Might want to change to be ~5000 mL of blood (realistic levels)
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Needs", meta = (AllowPrivateAccess = true))
	FDEStat Satiation = FDEStat(100, 100, -0.125);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Needs", meta = (AllowPrivateAccess = true))
	FDEStat Hydration = FDEStat(100, 100, -0.25);
#pragma endregion
	
	void TickStats(const float& DeltaTime);
	void TickStamina(const float& DeltaTime);
	void TickSatiation(const float& DeltaTime);
	void TickHydration(const float& DeltaTime);
	void TickHealth(const float& DeltaTime);
	void TickBlood(const float& DeltaTime);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = true))
	bool bIsSprinting = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = true))
	bool bIsCrouching = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = true))
	float SprintCostMultiplier = 2;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = true))
	float JumpCost = 10;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float HealthRegenDelay = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float CurrentStaminaExhaustion = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float SecondsForStaminaExhaustion = 5.f;
	
	bool bIsExhausted = false;
	
	void HandleStarvationStart(AActor* Actor);
	void HandleStarvationEnd(AActor* Actor);
	void HandleDehydrationStart(AActor* Actor);
	void HandleDehydrationEnd(AActor* Actor);
	void HandleLowBloodStart(AActor* Actor);
	void HandleLowBloodEnd(AActor* Actor);
	
	void HandleSprintStart(AActor* Actor);
	void HandleSprintEnd(AActor* Actor);
	void HandleCrouchStart(AActor* Actor);
	void HandleCrouchEnd(AActor* Actor);
	void HandleFallingStart(AActor* Actor);
	void HandleFallingEnd(AActor* Actor);
	
	
	bool bIsStarving = false;
	bool bIsDehydrated = false;
	bool bHasLowBlood = false;
	
	bool bIsFalling = false;
};
