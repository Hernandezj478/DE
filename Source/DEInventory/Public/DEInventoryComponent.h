// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DEInventorySlot.h"
#include "DEInventoryInterface.h"
#include "DEInventoryComponent.generated.h"

struct FDEItemData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEINVENTORY_API UDEInventoryComponent : public UActorComponent, public IDEInventoryInterface
{
	GENERATED_BODY()

public:
	UDEInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void ResizeInventory();
	virtual void UpdateWeight();
	bool UpdateInventorySlotCount(const int32 NewSlots);
	float GetCarryWeightPercentile() const {return CurrentCarryWeight / MaxCarryWeight;}
	
	// Inventory mutable functions
	// int32 AddItemToInventory(const FDEItemData& ItemData, const int32 Quantity, const float Durability = -1.f);
	// bool RemoveItemFromInventory(const FName ItemID, const int32 Quantity);
	// bool TransferItemFromInventory(UDEInventoryComponent* TargetInventory, const FName& ItemID, const int32 Quantity, const int TargetIndex = -1);
	// int32 AddItemToSlot(const FDEItemData& ItemData, const int32 Quantity, const float Durability = -1.f, const int Index = -1);
	
	/*
	 * Interface function implementation
	 */
	virtual bool RemoveItem_Implementation(const FName& ItemID, const int32& Quantity) override;
	virtual int32 AddItem_Implementation(const FName& ItemID, const int32& Quantity, const float& Durability) override;
	virtual bool TransferItem_Implementation(const FName& ItemID, const int32& Quantity, 
		const TScriptInterface<IDEInventoryInterface>& TargetInventory) override;
	
	virtual bool RemoveItemFromSlot_Implementation(const int32& Index, const int32& Quantity) override;
	virtual int32 AddItemToSlot_Implementation(const int32& Index, const FName& ItemID, const int32& Quantity, const float& Durability) override;
	virtual bool TransferItemFromSlot_Implementation(const int32& Index, const int32& Quantity, 
		const TScriptInterface<IDEInventoryInterface>& TargetInventory) override;
	
	
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	int32 TotalNumberOfSlots = 10;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TArray<FDEInventorySlot> InventoryContents;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float MaxCarryWeight = 100.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float CurrentCarryWeight = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	UDataTable* ItemDataTable = nullptr;
	
	// void DropItemToWorldAtIndex(const int Index);
	// void DropItemFromSlotData(const FDEInventorySlot& SlotData);
	
	bool ExtractItemFromSlot(int32 Index, int32 Quantity, FDEInventorySlot& OutExtracted);
	const FDEItemData* GetItemData(FName ItemID) const;

};
