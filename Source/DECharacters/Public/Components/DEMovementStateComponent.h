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

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	bool CanSprint() const;
	bool CanJump() const;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	bool bCanSprint = true;
	bool bCanJump = true;
	
	// Event handlers
	void HandleExhaustionStart(AActor* Actor);
	void HandExhaustionEnd(AActor* Actor);
	
	// Cached owner
	AActor* OwnerActor = nullptr;
};
