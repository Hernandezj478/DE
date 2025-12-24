// Fill out your copyright notice in the Description page of Project Settings.


#include "DENPCCharacterBase.h"


// Sets default values
ADENPCCharacterBase::ADENPCCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADENPCCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADENPCCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ADENPCCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

