// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

class UStatComponent;
class UMovementStateComponent;
class UInventoryComponent;

UCLASS(Abstract, NotBlueprintable)
class CHARACTERS_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

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
	UMovementStateComponent* MovementStateComponent;

#pragma region Movement
	bool CanCharacterJump() const;
	void CharacterJump();

	void SetSprinting(const bool& bSprinting);
	void SetCrouch(const bool& bCrouch);

#pragma endregion Movement

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	UStatComponent* Statline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = true))
	UInventoryComponent* InventoryComponent;
};
