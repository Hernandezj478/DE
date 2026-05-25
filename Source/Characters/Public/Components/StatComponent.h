// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EStatTypes.h"
#include "FStat.h"
#include "SaveableInterface.h"
#include "StatComponent.generated.h"

class ACharacterBase;
class UMessagingSubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CHARACTERS_API UStatComponent : public UActorComponent, public ISaveableInterface
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

	virtual FName GetSaveID_Implementation() const override;
	virtual bool CollectSaveData_Implementation(FEntitySaveRecord& OutRecord) const override;
	virtual void ApplySaveData_Implementation(const FEntitySaveRecord& Record) override;
	virtual void OnPreSave_Implementation() override;
	virtual void OnPostLoad_Implementation() override;
private:
	
#pragma region Stats
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FStat Health;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Stamina, Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FStat Stamina = FStat(100.f, 100.f, 5.f);
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Blood, Category = "Statline|Stats", meta = (AllowPrivateAccess = true))
	FStat Blood = FStat(5000.f, 5000.f, 10.f);
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Satiation, Category = "Statline|Stats|Needs", meta = (AllowPrivateAccess = true))
	FStat Satiation = FStat(100, 100, -0.125);
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Hydration, Category = "Statline|Stats|Needs", meta = (AllowPrivateAccess = true))
	FStat Hydration = FStat(100, 100, -0.25);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Hydration, Category = "Statline|Stats|Needs", meta = (AllowPrivateAccess = true))
	FStat Fatigue = FStat(100, 100, -0.125);

	UPROPERTY(VisibleAnywhere, Category = "Statline|Stats|Modifiers", meta = (AllowedPrivateAccess = true))
	bool bIsExhausted = false;
#pragma endregion

#pragma region Movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = "true"))
	float SprintCostMultiplier = 2.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Movement", meta = (AllowPrivateAccess = "true"))
	float JumpCost = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float HealthRegenDelay = 0.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float CurrentStaminaExhaustion = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float SecondsForStaminaExhaustion = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float ExhaustionThreashold = 10.f;
#pragma endregion
	UMessagingSubsystem* pMessanger;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float LowBloodThreshold = 40.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float MaxHealthDrainRate = 3.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float HealthDrainRate = 1.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	float StaminaExhaustedDrainRate = 1.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	int DaysToStarvation = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Statline|Stats|Modifiers", meta = (AllowPrivateAccess = "true"))
	int DaysToDehydration = 3;

	bool bIsStarving = false;
	bool bIsDehydrated = false;
	bool bHasLowBlood = false;

	void CalculateTickRate();

	void TickStats(const float DeltaTime);
	void TickHealth(const float DeltaTime);
	void TickStamina(const float DeltaTime);
	void TickBlood(const float DeltaTime);
	void TickSatiation(const float DeltaTime);
	void TickHydration(const float DeltaTime);
	void TickFatigue(const float DeltaTime);

#pragma region RepNotifies
	UFUNCTION()
	void OnRep_Health();
	UFUNCTION()
	void OnRep_Stamina();
	UFUNCTION()
	void OnRep_Blood();
	UFUNCTION()
	void OnRep_Satiation();
	UFUNCTION()
	void OnRep_Hydration();
#pragma endregion
	void SetIsExhausted(bool bNewValue);
};

inline ACharacterBase* UStatComponent::GetCharacterOwner() const
{
	return CharacterOwner;
}
