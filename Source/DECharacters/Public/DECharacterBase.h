// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DECharacterBase.generated.h"

class UDEStatComponent;
class UDEMovementStateComponent;
class UDEInventoryComponent;

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
	
	UFUNCTION(BlueprintCallable)
	UActorComponent* GetCharacterInventory() const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = true))
	UDEMovementStateComponent* MovementStateComponent;

#pragma region Movement
	bool CanCharacterJump() const;
	void CharacterJump();
	
	void SetSprinting(const bool& bSprinting);
	void SetCrouch(const bool& bCrouch);
	
#pragma endregion Movement
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	UDEStatComponent* Statline;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = true))
	UDEInventoryComponent* InventoryComponent;
};
