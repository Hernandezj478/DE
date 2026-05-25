// Fill out your copyright notice in the Description page of Project Settings.
#include "InventoryComponent.h"
#include "ItemData.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

bool UInventoryComponent::UpdateInventorySlotCount(const int32 NewSlots)
{
	TotalNumberOfSlots = NewSlots;
	ResizeInventory();
	UpdateWeight();
	return true;
}

bool UInventoryComponent::RemoveItem_Implementation(const FName& ItemID, const int32& Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0)
	{
		return false;
	}
	int32 Remaining = Quantity;
	for (int32 Index = InventoryContents.Num() - 1; Index >= 0; Index--)
	{
		if (Remaining <= 0)
		{
			break;
		}
		FInventorySlot& Slot = InventoryContents[Index];
		
		// Skip empty slots or non-matching items
		if (!Slot.IsValid() || Slot.ItemID != ItemID)
		{
			continue;
		}
		FInventorySlot Extracted;
		if (ExtractItemFromSlot(Index, Remaining, Extracted))
		{
			Remaining -= Extracted.Quantity;
		}
	}
	// Success only if everything was removed as requested
	UpdateWeight();
	return Remaining == 0;
}

int32 UInventoryComponent::AddItem_Implementation(const FName& ItemID, const int32& Quantity, const float& Durability)
{
	if (Quantity <= 0)
	{
		return Quantity;
	}
	const FItemData* ItemData = GetItemData(ItemID);
	if (!ItemData || !ItemData->IsValid())
	{
		return Quantity;
	}
	int32 Remaining = Quantity;
	// Stack into existing stacks
	for (FInventorySlot& Slot : InventoryContents)
	{
		if (Remaining <= 0)
		{
			break;
		}
		// Empty slot, move on
		if (!Slot.IsValid())
		{
			continue;
		}
		// not the same item, move on
		if (Slot.ItemID != ItemData->ItemID)
		{
			continue;
		}
		// full sack, move on
		if (Slot.Quantity >= ItemData->ItemMaxStackSize)
		{
			continue;
		}
		const int32 Space = ItemData->ItemMaxStackSize -  Slot.Quantity;
		const int32 ToAdd = FMath::Min(Space, Remaining);
		
		Slot.Quantity += ToAdd;
		Remaining -= ToAdd;
	}
	// Fill empty slots
	for (FInventorySlot& Slot : InventoryContents)
	{
		if (Remaining <= 0)
		{
			break;
		}
		// occupied slot, move on
		if (Slot.IsValid())
		{
			continue;
		}
		const int32 ToAdd = FMath::Min(ItemData->ItemMaxStackSize, Remaining);
		
		Slot.SetItemData(ItemData->ItemID, ToAdd, Durability, ItemData->SingleItemWeight);
		Remaining -= ToAdd;
	}
	UpdateWeight();
	return Remaining;
}

bool UInventoryComponent::TransferItem_Implementation(const FName& ItemID, const int32& Quantity,
                                                        const TScriptInterface<IInventoryInterface>& TargetInventory)
{
	// Is ItemID valid, target valid, ItemID valid, valid quantity
	if (!TargetInventory.GetObject() || !GetItemData(ItemID) || Quantity <= 0)
	{
		return false;
	}
	int32 Transferable = TargetInventory->Execute_GetAvailableSpace(TargetInventory.GetObject(), ItemID, Quantity);
	if (Transferable <= 0)
	{
		return false;
	}
	int32 TotalTransferred = 0;
	int32 Remaining = Transferable;
	for (int i = InventoryContents.Num() - 1; i >= 0; i--)
	{
		// we need to find the item given
		if (ItemID != InventoryContents[i].ItemID)
		{
			continue;
		}
		FInventorySlot ExtractedItem;
		ExtractItemFromSlot(i, Remaining, ExtractedItem);
		TargetInventory->Execute_AddItem(TargetInventory.GetObject(), ExtractedItem.ItemID, 
			ExtractedItem.Quantity, ExtractedItem.CurrentDurability);
		Remaining -= ExtractedItem.Quantity;
		TotalTransferred += ExtractedItem.Quantity;
		
	}
	
	if (TotalTransferred > 0)
	{
		UpdateWeight();
		return true;
	}
	return false;
}

bool UInventoryComponent::RemoveItemFromSlot_Implementation(const int32& Index, const int32& Quantity)
{
	FInventorySlot ExtractedItem;
	if (!ExtractItemFromSlot(Index, Quantity, ExtractedItem))
	{
		return false;
	}
	UpdateWeight();
	return ExtractedItem.Quantity > 0;
}

int32 UInventoryComponent::AddItemToSlot_Implementation(const int32& Index, const FName& ItemID, const int32& Quantity,
	const float& Durability)
{
	if (!InventoryContents.IsValidIndex(Index) || Quantity <= 0)
	{
		return Quantity;
	}
	const FItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		return Quantity;
	}
	
	FInventorySlot& Slot = InventoryContents[Index];
	if (Slot.IsValid() && Slot.ItemID != ItemID)
	{
		return Quantity;
	}
	if (!Slot.IsValid())
	{
		const int32 ToAdd = FMath::Min(ItemData->ItemMaxStackSize, Quantity);
		Slot.SetItemData(ItemID, ToAdd, Durability, ItemData->SingleItemWeight);
		UpdateWeight();
		return Quantity - ToAdd;
	}
	
	const int32 Capacity = ItemData->ItemMaxStackSize - Slot.Quantity;
	if (Capacity <= 0)
	{
		return Quantity;
	}
	
	const int32 ToAdd = FMath::Min(Capacity, Quantity);
	Slot.Quantity += ToAdd;
	UpdateWeight();
	return Quantity - ToAdd;
}

bool UInventoryComponent::TransferItemFromSlot_Implementation(const int32& Index, const int32& Quantity,
	const TScriptInterface<IInventoryInterface>& TargetInventory)
{
	if (!InventoryContents.IsValidIndex(Index) || !TargetInventory.GetObject() || Quantity <= 0)
	{
		return false;
	}
	const FInventorySlot& Slot = InventoryContents[Index];
	if (!Slot.IsValid())
	{
		return false;
	}
	int32 Transferable = TargetInventory->Execute_GetAvailableSpace(TargetInventory.GetObject(), Slot.ItemID, Quantity);
	if (Transferable <= 0)
	{
		return false;
	}
	FInventorySlot ExtractedItem;
	ExtractItemFromSlot(Index, Transferable, ExtractedItem);
	TargetInventory->Execute_AddItem(TargetInventory.GetObject(), ExtractedItem.ItemID, ExtractedItem.Quantity, ExtractedItem.CurrentDurability);
	UpdateWeight();
	return true;
}

int32 UInventoryComponent::GetAvailableSpace_Implementation(const FName& ItemID, const int32& Quantity) const
{
	const FItemData* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		return 0;
	}
	int32 Space = 0;
	for (const FInventorySlot& Slot : InventoryContents)
	{
		if (!Slot.IsValid() || Slot.ItemID != ItemID)
		{
			continue;
		}
		Space += ItemData->ItemMaxStackSize - Slot.Quantity;
	}
	for (const FInventorySlot& Slot : InventoryContents)
	{
		if (Slot.IsValid())
		{
			continue;
		}
		Space += ItemData->ItemMaxStackSize;
	}
	return FMath::Min(Space, Quantity);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ResizeInventory();
	UpdateWeight();
}

void UInventoryComponent::ResizeInventory()
{
	if (InventoryContents.Num() == TotalNumberOfSlots)
	{
		return;
	}
	if (InventoryContents.Num() < TotalNumberOfSlots)
	{
		while (InventoryContents.Num() < TotalNumberOfSlots)
		{
			InventoryContents.Add(FInventorySlot());
		}
		UpdateWeight();
		return;
	}
	while (InventoryContents.Num() > TotalNumberOfSlots)
	{
		FInventorySlot DroppedItem = InventoryContents.Pop();
		if (DroppedItem.IsValid())
		{
			// TODO: Drop extra items in the world
			//DropItemFromSlotData(DroppedItem);
		}
	}
	UpdateWeight();
}

void UInventoryComponent::UpdateWeight()
{
	// TODO: create event on bus to signal if weight is over max carry capacity.
	float TempWeight = 0.f;
	for (const FInventorySlot& Item : InventoryContents)
	{
		if (!Item.IsValid())
		{
			continue;
		}
		TempWeight += (Item.Quantity * Item.ItemWeight);
	}
	CurrentCarryWeight = TempWeight;
}

// void UInventoryComponent::DropItemToWorldAtIndex(const int Index)
// {
// 	//TODO: call world spawn manager to spawn dropped item
// }
//
// void UInventoryComponent::DropItemFromSlotData(const FInventorySlot& SlotData)
// {
// 	
// }

bool UInventoryComponent::ExtractItemFromSlot(int32 Index, int32 Quantity, FInventorySlot& OutExtracted)
{
	OutExtracted.ClearItemData();
	if (Quantity <= 0)
	{
		return false;
	}
	// Make sure we are in range of InventoryContents
	if (!InventoryContents.IsValidIndex(Index))
	{
		return false;
	}

	FInventorySlot& Slot = InventoryContents[Index];
	// Check if not empty
	if (!Slot.IsValid())
	{
		return false;
	}
	 const int32 ExtractedQuantity = FMath::Min(Slot.Quantity, Quantity);
	
	OutExtracted = Slot;
	OutExtracted.Quantity = ExtractedQuantity;
	
	Slot.Quantity -= ExtractedQuantity;
	// If slot emptied, clear data
	if (!Slot.IsValid())
	{
		Slot.ClearItemData();
	}
	return true;
}

const FItemData* UInventoryComponent::GetItemData(FName ItemID) const
{
	if (ItemID.IsNone() || !ItemDataTable)
	{
		return nullptr;
	}
	return ItemDataTable->FindRow<FItemData>(ItemID, TEXT("GetItemData"), false);
}
