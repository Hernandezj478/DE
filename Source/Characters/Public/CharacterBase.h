// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/DECharacterMovementComponent.h"
#include "CharacterBase.generated.h"

class UStatComponent;
class UMovementStateComponent;
class UInventoryComponent;

UCLASS(Abstract, NotBlueprintable)
class CHARACTERS_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, replicatedUsing = OnRep_IsSprinting, Category = Character)
	bool bIsSprinting = false;
	UPROPERTY(BlueprintReadOnly, replicatedUsing = OnRep_IsRunning, Category = Character)
	bool bIsRunning = false;
	UPROPERTY(BlueprintReadOnly)
	float CharacterMass = 80.f; // in kG

	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	UActorComponent* GetCharacterInventory() const;
	
	UFUNCTION()
	virtual void OnRep_IsSprinting();
	UFUNCTION()
	virtual void OnRep_IsRunning();

	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;

	virtual void Run();
	virtual void StopRun();
	virtual void Sprint();
	virtual void StopSprint();
	bool CanSprint();
	bool CanRun();
	
	void SetIsSprinting(bool IsSprinting);
	void SetIsRunning(bool IsRunning);

	bool IsSprinting() const;
	bool IsRunning() const;
	bool IsFalling() const;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region Movement
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;
#pragma endregion Movement

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	UStatComponent* Statline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = true))
	UInventoryComponent* InventoryComponent;
};

inline bool ACharacterBase::IsSprinting() const
{
	return bIsSprinting;
}

inline bool ACharacterBase::IsRunning() const
{
	return bIsRunning;
}

inline bool ACharacterBase::IsFalling() const
{
	return GetMovementComponent()->IsFalling();
}