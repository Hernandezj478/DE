// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EStatTypes.h"
#include "FStat.h"
#include "StatComponent.generated.h"

class ACharacterBase;
class UMessagingSubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHARACTERS_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStatComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	float GetStatPercentile(const EStatTypes Stat) const;
	
	UFUNCTION(BlueprintCallable)
	bool CanJump();
	UFUNCTION(BlueprintCallable)
	bool CanSprint();
	UFUNCTION(BlueprintCallable)
	bool CanRun();

	UFUNCTION(BlueprintCallable)
	void ConsumeJumpStamina();
	
	UFUNCTION(BlueprintCallable)
	void AdjustStat(const EStatTypes Stat, const float& Amount);

	UFUNCTION()
	ACharacterBase* GetCharacterOwner() const;

protected:

	TObjectPtr<ACharacterBase> CharacterOwner;

	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
#pragma region Stats
	/*
	* Stat tick rate can be adjusted through buffs/debufs and skills/perks
	* @see FStat::AdjustTick
	*/
	// =========================================================
	// Replicated Stats
	// =========================================================
	// Health: Replicated to all - other players need it for healing/damage
	// Health will increase capacity as player progresses and with skills/perks
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FStat Health;

	// Stamina: owner only - server authoritive, client predicts locally
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina ,Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FStat Stamina = FStat(100.f, 100.f, 5.f);
	
	// Blood: Owner only - only affects owning plaer's debuffs
	// Blood should only tick up when right conditions are met
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Blood ,Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FStat Blood = FStat(5000.f, 5000.f, 0.f);	// Might want to change to be ~5000 mL of blood (realistic levels)
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_IsExhausted, Category = "Statline|Stats|Modifiers", meta = (AllowedPrivateAccess = true))
	bool bIsExhausted = false;

	// =========================================================
	// Local Only Stats (never replicated)
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Needs", meta = (AllowPrivateAccess = true))
	FStat Satiation = FStat(100, 100, -0.125);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Needs", meta = (AllowPrivateAccess = true))
	FStat Hydration = FStat(100, 100, -0.25);

	// =========================================================
	// Predicted Stamina (client side only)
	// Drives UI immediately without waiting for server replication
	// =========================================================
	float PredictedStamina = 0.f;

#pragma endregion

#pragma region Movement
	// =========================================================
	// Movement cost settings
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = true))
	float SprintCostMultiplier = 2.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = true))
	float JumpCost = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float HealthRegenDelay = 0.f;
	// =========================================================
	// Exhaustion Settings
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float CurrentStaminaExhaustion = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float SecondsForStaminaExhaustion = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float ExhaustionThreashold = 10.f;
#pragma endregion
	UMessagingSubsystem* pMessanger;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float MaxHealthDrainRate = 3.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = true))
	float HealthDrainRate = 1.5f;
	// =========================================================
	// State Flags (local, derived from replicated values)
	// =========================================================
	bool bIsStarving = false;
	bool bIsDehydrated = false;
	bool bHasLowBlood = false;
	// =========================================================
	// Tick Helpers
	// =========================================================
	void TickStats(const float& DeltaTime);		// Server only
	void TickStaminaPredicted(float DeltaTime); // AutonomousProxy only
	void TickStamina(const float& DeltaTime);
	void TickSatiation(const float& DeltaTime);
	void TickHydration(const float& DeltaTime);
	void TickHealth(const float& DeltaTime);	// Local only
	void TickBlood(const float& DeltaTime);		// Local only

#pragma region NetworkRep
	// =========================================================
	// Replication Notifies
	// =========================================================
	UFUNCTION()
	void OnRep_Stamina();
	UFUNCTION()
	void OnRep_Blood();
	UFUNCTION()
	void OnRep_IsExhausted();
#pragma endregion
	// =========================================================
	// State Setters
	// =========================================================
	// Single point of truth for state changes - handles both
	// server replication and lister server manual notify
	void SetIsExhausted(bool bNewValue);
};

inline ACharacterBase* UStatComponent::GetCharacterOwner() const
{
	return CharacterOwner;
}
