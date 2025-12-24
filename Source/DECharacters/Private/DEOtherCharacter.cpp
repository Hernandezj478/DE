// Fill out your copyright notice in the Description page of Project Settings.


#include "DEOtherCharacter.h"


// Sets default values
ADEOtherCharacter::ADEOtherCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADEOtherCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADEOtherCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADEOtherCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

