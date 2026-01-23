// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DEInventorySlot.h"
#include "DEInventoryComponent.generated.h"

struct FDEItemData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEINVENTORY_API UDEInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDEInventoryComponent();
	
	UFUNCTION(BlueprintCallable)
	bool UpdateInventorySlotCount(const int32 NewSlots);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetCarryWeightPercentile() const {return CurrentCarryWeight / MaxCarryWeight;}
	
	UFUNCTION(BlueprintCallable)
	int32 AddItemToInventory(const FDEItemData& ItemData, const int32 Quantity, const float Durability = -1.f);
	
	UFUNCTION(BlueprintCallable)
	bool RemoveItemFromInventory(const FName ItemID, const int32 Quantity);
	
	UFUNCTION(BlueprintCallable)
	void TransferItemFromInventory(){}
	
	UFUNCTION(BlueprintCallable)
	int32 AddItemToSlot(const FDEItemData& ItemData, const int32 Quantity, const float Durability = -1.f, const int Index = -1);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void ResizeInventory();
	virtual void UpdateWeight();
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	int32 TotalNumberOfSlots = 10;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TArray<FDEInventorySlot> InventoryContents;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float MaxCarryWeight = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float CurrentCarryWeight = 0.f;
	
	void DropItemToWorldAtIndex(const int Index);
	void DropItemFromSlotData(const FDEInventorySlot& SlotData);
};
