// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DEMovementStateComponent.h"
#include "DEEventBus.h"

// Sets default values for this component's properties
UDEMovementStateComponent::UDEMovementStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UDEMovementStateComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerActor = GetOwner();
	
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnExhaustionStart.AddUObject(this, &UDEMovementStateComponent::HandleExhaustionStart);
		Bus->OnExhaustionEnd.AddUObject(this, &UDEMovementStateComponent::HandExhaustionEnd);
	}
	
}

void UDEMovementStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (UDEEventBus* Bus = UDEEventBus::Get())
	{
		Bus->OnExhaustionStart.RemoveAll(this);
		Bus->OnExhaustionEnd.RemoveAll(this);
	}
}

void UDEMovementStateComponent::HandleExhaustionStart(AActor* Actor)
{
	if (Actor != OwnerActor)
	{
		return;
	}
	bCanSprint = false;
	bCanJump = false;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Exhaustion Start"));
	}
}

void UDEMovementStateComponent::HandExhaustionEnd(AActor* Actor)
{
	if (Actor != OwnerActor)
	{
		return;
	}
	bCanSprint = true;
	bCanJump = true;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Exhaustion End"));
	}
}


// Called every frame
void UDEMovementStateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UDEMovementStateComponent::CanSprint() const
{
	return bCanSprint;
}

bool UDEMovementStateComponent::CanJump() const
{
	return bCanJump;
}

