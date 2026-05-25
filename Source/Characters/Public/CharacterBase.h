// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/DECharacterMovementComponent.h"
#include "VoxelInterface.h"
#include "SaveableInterface.h"
#include "CharacterBase.generated.h"

class UStatComponent;
class UMovementStateComponent;
class UInventoryComponent;
class AVoxelWorldActor;

UCLASS(Abstract, NotBlueprintable)
class CHARACTERS_API ACharacterBase : public ACharacter, public IVoxelInterface, public ISaveableInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsSprinting, Category = "Character")
	bool bIsSprinting = false;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsRunning, Category = "Character")
	bool bIsRunning = false;
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsOverEncumbered, Category = "Character")
	bool bIsOverEncumbered = false;
	UPROPERTY(BlueprintReadOnly)
	float CharacterMass = 80.f; // in kG
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	virtual void OnRep_Controller() override;

	UFUNCTION(BlueprintCallable)
	UActorComponent* GetCharacterInventory() const;
	
	UFUNCTION()
	virtual void OnRep_IsSprinting();
	UFUNCTION()
	virtual void OnRep_IsRunning();
	UFUNCTION()
	virtual void OnRep_IsOverEncumbered();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DebugAdjustStat(const EStatTypes Stat, float Amount);
	void Server_DebugAdjustStat_Implementation(const EStatTypes Stat, float Amount);

	void UpdateEncumberanceState();

	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;

	virtual void Run();
	virtual void StopRun();
	virtual void Sprint();
	virtual void StopSprint();
	bool CanSprint();
	bool CanRun();
	
	void SetIsSprinting(bool IsSprinting);
	void SetIsRunning(bool IsRunning);

	bool IsSprinting() const;
	bool IsRunning() const;
	bool IsFalling() const;

	float GetCarryWeightPercentile() const;

	// Terrain Interface
	UFUNCTION(BlueprintCallable)
	virtual void RequestTerrainDig(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength) override;
	UFUNCTION(Blueprintcallable)
	virtual void RequestTerrainAdd(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Save")
	FName SaveID = NAME_None;
	FGuid PersistentSaveGUID;
	bool bIsRuntimeSpawned = false;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
#pragma region Movement
	virtual bool CanJumpInternal_Implementation() const override;
	virtual void OnJumped_Implementation() override;
#pragma endregion Movement

	virtual FName GetSaveID_Implementation() const override;
	virtual bool CollectSaveData_Implementation(FEntitySaveRecord& OutRecord) const override;
	virtual void ApplySaveData_Implementation(const FEntitySaveRecord& Record) override;
	virtual void OnPreSave_Implementation() override;
	virtual void OnPostLoad_Implementation() override;

private:
	static const FString IDSlotName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	UStatComponent* Statline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = true))
	UInventoryComponent* InventoryComponent;

	void InitializeID();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRegisterID(FGuid InGUID);
	bool ServerRegisterID_Validate(FGuid InGUID);
	void ServerRegisterID_Implementation(FGuid InGUID);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestDig(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength);
	bool ServerRequestDig_Validate(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength);
	void ServerRequestDig_Implementation(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRequestAdd(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength);
	bool ServerRequestAdd_Validate(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength);
	void ServerRequestAdd_Implementation(AVoxelWorldActor* TerrainActor, FVector WorldCenter, float Radius, float Strength);

	void SetIsOverEncumbered(bool NewEncumberance);
};

inline bool ACharacterBase::IsSprinting() const
{
	return bIsSprinting;
}

inline bool ACharacterBase::IsRunning() const
{
	return bIsRunning;
}

inline bool ACharacterBase::IsFalling() const
{
	return GetMovementComponent()->IsFalling();
}