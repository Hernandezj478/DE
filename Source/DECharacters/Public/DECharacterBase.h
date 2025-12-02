// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DECharacterBase.generated.h"

UCLASS(Abstract, NotBlueprintable)
class DECHARACTERS_API ADECharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADECharacterBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
#pragma region Movement
	bool CanCharacterJump() const;
	void HasJumped();
	
	float GetSneakSpeed() const;
	float GetWalkSpeed() const;
	float GetSprintSpeed() const;
	
	void SetSprinting(const bool bSprinting);
	void SetSneaking(const bool bSneaking);
#pragma endregion Movement
	
private:
#pragma region Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	float SneakSpeed = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	float WalkSpeed = 200.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	float SprintSpeed = 400.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	bool bIsSprinting = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	bool bIsSneaking = false;
#pragma endregion Movement
	
	
};
