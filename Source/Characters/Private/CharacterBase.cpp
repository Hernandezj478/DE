// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "Components/StatComponent.h"
#include "InventoryComponent.h"
#include "Logger.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer.SetDefaultSubobjectClass<UDECharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Enable replication
	SetReplicates(true);
	//bReplicates = true;
	//bAlwaysRelevant = true;
	GetCharacterMovement()->SetIsReplicated(true);
	GetMesh()->SetIsReplicated(true);
	Statline = CreateDefaultSubobject<UStatComponent>(TEXT("Statline"));
	if (!IsValid(Statline))
	{
		Logger::GetInstance()->AddMessage("Statline has not been created/initialized", ERROR);
	}
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

bool ACharacterBase::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && (Statline && Statline->CanJump()) ;
}

void ACharacterBase::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (Statline && HasAuthority())
	{
		Statline->ConsumeJumpStamina();
	}
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACharacterBase, bIsSprinting);
	DOREPLIFETIME(ACharacterBase, bIsRunning);
}

UActorComponent* ACharacterBase::GetCharacterInventory() const
{
	return InventoryComponent;
}

void ACharacterBase::OnRep_IsSprinting()
{
	if (UDECharacterMovementComponent* characterMovement = GetCharacterMovement<UDECharacterMovementComponent>())
	{
		if (IsSprinting())
		{
			characterMovement->bWantsToSprint = true;
			characterMovement->Sprint(true);
		}
		else
		{
			characterMovement->bWantsToSprint = false;
			characterMovement->StopSprint(true);
		}
		characterMovement->bNetworkUpdateReceived = true;
	}
}

void ACharacterBase::OnRep_IsRunning()
{
	if (UDECharacterMovementComponent* characterMovement = GetCharacterMovement<UDECharacterMovementComponent>())
	{
		if (IsRunning())
		{
			characterMovement->bWantsToRun = true;
			characterMovement->Run(true);
		}
		else
		{
			characterMovement->bWantsToRun = false;
			characterMovement->StopRun(true);
		}
		characterMovement->bNetworkUpdateReceived = true;
	}
}

void ACharacterBase::Server_DebugAdjustStat_Implementation(const EStatTypes Stat, float Amount)
{
	if (Statline)
	{
		Statline->AdjustStat(Stat, Amount);
	}
}

void ACharacterBase::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);
}

void ACharacterBase::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);
}

void ACharacterBase::Run()
{
	if (GetCharacterMovement())
	{
		if (CanRun())
		{
			GetCharacterMovement<UDECharacterMovementComponent>()->bWantsToRun = true;
		}
	}
}

void ACharacterBase::StopRun()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement<UDECharacterMovementComponent>()->bWantsToRun = false;
	}
}

void ACharacterBase::Sprint()
{
	if (GetCharacterMovement())
	{
		if (CanSprint())
		{
			GetCharacterMovement<UDECharacterMovementComponent>()->bWantsToSprint = true;
		}
	}
}

void ACharacterBase::StopSprint()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement<UDECharacterMovementComponent>()->bWantsToSprint = false;
	}
}

void ACharacterBase::SetIsSprinting(bool IsSprinting)
{
	if(bIsSprinting == IsSprinting)
	{
		return;
	}
	bIsSprinting = IsSprinting;
	OnRep_IsSprinting();
}

void ACharacterBase::SetIsRunning(bool IsRunning)
{
	if (bIsRunning == IsRunning)
	{
		return;
	}
	bIsRunning = IsRunning;
	OnRep_IsRunning();
}

bool ACharacterBase::CanSprint()
{
	return Statline && Statline->CanSprint();
}

bool ACharacterBase::CanRun()
{
	// Running does not drain stamina, but will cause thirst/hunger to drain faster
	return Statline && Statline->CanRun();
}
