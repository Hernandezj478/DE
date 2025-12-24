// Fill out your copyright notice in the Description page of Project Settings.


#include "DEAnimalCharacter.h"


// Sets default values
ADEAnimalCharacter::ADEAnimalCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADEAnimalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADEAnimalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADEAnimalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

