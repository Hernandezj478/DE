// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "Components/StatComponent.h"
#include "InventoryComponent.h"
#include "VoxelWorldActor.h"
#include "Logger.h"
#include "Net/UnrealNetwork.h"
#include "SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

const FString ACharacterBase::IDSlotName = TEXT("PlayerID");

// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer.SetDefaultSubobjectClass<UDECharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Enable replication
	SetReplicates(true);
	GetCharacterMovement()->SetIsReplicated(true);
	GetMesh()->SetIsReplicated(true);
	Statline = CreateDefaultSubobject<UStatComponent>(TEXT("Statline"));
	if (!IsValid(Statline))
	{
		LOG_ERROR(LogCharacters, "Statline has not been created/initialized");
	}
	PersistentSaveGUID = FGuid::NewGuid();
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacterBase::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
}

bool ACharacterBase::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && (Statline && Statline->CanJump());
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
	DOREPLIFETIME(ACharacterBase, bIsOverEncumbered);
}

void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!HasAuthority())
	{
		return;
	}
	if (IsPlayerControlled())
	{
		// Player registeration deferred - need client to send PersistantSaveGUID via ServerRegisterID
		return;
	}
	if (USaveGameSubsystem* SaveSys = GetWorld()->GetSubsystem<USaveGameSubsystem>())
	{
		SaveSys->RegisterSaveable(this);
	}
}

void ACharacterBase::UnPossessed()
{
	if (HasAuthority())
	{
		if (IsPlayerControlled())
		{
			if (USaveGameSubsystem* SaveSys = GetWorld()->GetSubsystem<USaveGameSubsystem>())
			{
				SaveSys->UnregisterSaveable(this);
			}
		}
	}
	Super::UnPossessed();
}

void ACharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();
	InitializeID();
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

void ACharacterBase::OnRep_IsOverEncumbered()
{

}

void ACharacterBase::Server_DebugAdjustStat_Implementation(const EStatTypes Stat, float Amount)
{
	if (Statline)
	{
		Statline->AdjustStat(Stat, Amount);
	}
}

void ACharacterBase::UpdateEncumberanceState()
{
	if (!InventoryComponent)
	{
		return;
	}
	SetIsOverEncumbered(InventoryComponent->GetCarryWeightPercentile() >= 1.f);
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

float ACharacterBase::GetCarryWeightPercentile() const
{
	if (!InventoryComponent)
	{
		return 0.f;
	}
	return InventoryComponent->GetCarryWeightPercentile();
}

void ACharacterBase::RequestTerrainDig(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength)
{
	if (!TerrainActor)
	{
		return;
	}
	if (HasAuthority())
	{
		TerrainActor->DigSphere(WorldCenter, Radius, Strength);
	}
	else
	{
		ServerRequestDig(TerrainActor, WorldCenter, Radius, Strength);
	}
}

void ACharacterBase::InitializeID()
{
	if (!IsLocallyControlled() || !IsPlayerControlled())
	{
		return;
	}
	if (UDESaveGame* IDSave = Cast<UDESaveGame>(UGameplayStatics::LoadGameFromSlot(IDSlotName, 0)))
	{
		if (IDSave->PlayerGUID.IsValid())
		{
			ServerRegisterID(IDSave->PlayerGUID);
			return;
		}
	}
	FGuid NewGUID = FGuid::NewGuid();
	UDESaveGame* NewIDSave = Cast<UDESaveGame>(UGameplayStatics::CreateSaveGameObject(UDESaveGame::StaticClass()));
	NewIDSave->PlayerGUID = NewGUID;
	UGameplayStatics::SaveGameToSlot(NewIDSave, IDSlotName, 0);
	ServerRegisterID(NewGUID);
}

bool ACharacterBase::ServerRegisterID_Validate(FGuid InGUID)
{
	return InGUID.IsValid();
}

void ACharacterBase::ServerRegisterID_Implementation(FGuid InGUID)
{
	PersistentSaveGUID = InGUID;
	if (USaveGameSubsystem* SaveSys = GetWorld()->GetSubsystem<USaveGameSubsystem>())
	{
		SaveSys->RegisterSaveable(this);
	}
}

bool ACharacterBase::ServerRequestDig_Validate(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength)
{
	return TerrainActor != nullptr 
		&& Radius > 0.f && Radius <= 2000.f 
		&& Strength > 0.f && Strength <= 1.f;
}

void ACharacterBase::ServerRequestDig_Implementation(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength)
{
	TerrainActor->DigSphere(WorldCenter, Radius, Strength);
}

void ACharacterBase::RequestTerrainAdd(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength)
{
	if (!TerrainActor)
	{
		return;
	}
	if (HasAuthority())
	{
		TerrainActor->AddSphere(WorldCenter, Radius, Strength);
	}
	else
	{
		ServerRequestAdd(TerrainActor, WorldCenter, Radius, Strength);
	}
}

bool ACharacterBase::ServerRequestAdd_Validate(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength)
{
	return TerrainActor != nullptr
		&& Radius > 0.f && Radius <= 2000.f
		&& Strength > 0.f && Strength <= 1.f;
}

void ACharacterBase::ServerRequestAdd_Implementation(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength)
{
	TerrainActor->AddSphere(WorldCenter, Radius, Strength);
}

void ACharacterBase::SetIsOverEncumbered(bool NewEncumberance)
{
	if (bIsOverEncumbered == NewEncumberance)
	{
		return;
	}
	bIsOverEncumbered = NewEncumberance;
	OnRep_IsOverEncumbered();
}

FName ACharacterBase::GetSaveID_Implementation() const
{
	if (!SaveID.IsNone())
	{
		return SaveID;
	}
	return FName(PersistentSaveGUID.ToString());
}

bool ACharacterBase::CollectSaveData_Implementation(FEntitySaveRecord& OutRecord) const
{
	OutRecord.EntityID = GetSaveID();
	OutRecord.ActorTransform = GetActorTransform();
	OutRecord.bWasRuntimeSpawned = bIsRuntimeSpawned;
	OutRecord.ActorClass = GetClass();

	for (UActorComponent* Component : GetComponents())
	{
		if (!IsValid(Component) || !Component->Implements<USaveableInterface>())
		{
			continue;
		}
		FEntitySaveRecord ComponentRecord;
		if (ISaveableInterface::Execute_CollectSaveData(Component, ComponentRecord))
		{
			FSaveComponentData CompData;
			CompData.ComponentClass = Component->GetClass();
			CompData.ByteData = ComponentRecord.CustomData;
			CompData.RawData = ComponentRecord.RawData;
			OutRecord.ComponentData.Add(CompData);
		}
	}
	return true;
}

void ACharacterBase::ApplySaveData_Implementation(const FEntitySaveRecord& Record)
{
	if (bIsRuntimeSpawned && SaveID.IsNone())
	{
		if (!FGuid::Parse(Record.EntityID.ToString(), PersistentSaveGUID))
		{
			LOG_ERROR(LogCharacters, "Failed to parse GUID from EntityID '%s'", *Record.EntityID.ToString());
			return;
		}
	}
	SetActorTransform(Record.ActorTransform);
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
	}

	for (const FSaveComponentData& CompData : Record.ComponentData)
	{
		if (!IsValid(CompData.ComponentClass))
		{
			continue;
		}
		UActorComponent* FoundComponent = FindComponentByClass(CompData.ComponentClass);
		if (!IsValid(FoundComponent) || !FoundComponent->Implements<USaveableInterface>())
		{
			continue;
		}
		FEntitySaveRecord ComponentRecord;
		ComponentRecord.CustomData = CompData.ByteData;
		ComponentRecord.RawData = CompData.RawData;
		ISaveableInterface::Execute_ApplySaveData(FoundComponent, ComponentRecord);
		ISaveableInterface::Execute_OnPostLoad(FoundComponent);
	}
}

void ACharacterBase::OnPreSave_Implementation()
{

}

void ACharacterBase::OnPostLoad_Implementation()
{
	OnRep_IsSprinting();
	OnRep_IsRunning();
}
