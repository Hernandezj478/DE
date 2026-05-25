// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInventoryInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACE_API IInventoryInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Slot agnostic operations
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	int32 AddItem(const FName& ItemID, const int32& Quantity, const float& Durability);
	virtual int32 AddItem_Implementation(const FName& ItemID, const int32& Quantity, const float& Durability);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool RemoveItem(const FName& ItemID, const int32& Quantity);
	virtual bool RemoveItem_Implementation(const FName& ItemID, const int32& Quantity);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool TransferItem(const FName& ItemID, const int32& Quantity, const TScriptInterface<IInventoryInterface>& TargetInventory);
	virtual bool TransferItem_Implementation(const FName& ItemID, const int32& Quantity, const TScriptInterface<IInventoryInterface>& TargetInventory);
	
	// Slot specific operations
	UFUNCTION(BLueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	int32 AddItemToSlot(const int32& Index, const FName& ItemID, const int32& Quantity, const float& Durability);
	virtual int32 AddItemToSlot_Implementation(const int32& Index, const FName& ItemID, const int32& Quantity, const float& Durability);
	
	UFUNCTION(BLueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool RemoveItemFromSlot(const int32& Index, const int32& Quantity);
	virtual bool RemoveItemFromSlot_Implementation(const int32& Index, const int32& Quantity);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	bool TransferItemFromSlot(const int32& Index, const int32& Quantity, const TScriptInterface<IInventoryInterface>& TargetInventory);
	virtual bool TransferItemFromSlot_Implementation(const int32& Index, const int32& Quantity, const TScriptInterface<IInventoryInterface>& TargetInventory);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory")
	int32 GetAvailableSpace(const FName& ItemID, const int32& Quantity) const;
	virtual int32 GetAvailableSpace_Implementation(const FName& ItemID, const int32& Quantity) const;
};
