// Fill out your copyright notice in the Description page of Project Settings.


#include "DEHumanoidCharacter.h"


// Sets default values
ADEHumanoidCharacter::ADEHumanoidCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADEHumanoidCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADEHumanoidCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADEHumanoidCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

