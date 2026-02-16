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

// int32 UInventoryComponent::AddItemToInventory(const FItemData& ItemData, const int32 Quantity, const float Durability)
// {
// 	int32 NumberToAdd = Quantity;
// 	for (FInventorySlot& Item : InventoryContents)
// 	{
// 		if (!Item.IsValid() || ItemData.ItemID != Item.ItemID || Item.Quantity >= ItemData.ItemMaxStackSize)
// 		{
// 			continue;
// 		}
// 		int32 StackSpace = ItemData.ItemMaxStackSize - Item.Quantity;
// 		int32 ToAdd = FMath::Min(StackSpace, NumberToAdd);
// 		Item.Quantity += ToAdd;
// 		NumberToAdd -= ToAdd;
// 		if (NumberToAdd <= 0)
// 		{
// 			UpdateWeight();
// 			return NumberToAdd;	// Return number not added
// 		}
// 	}
// 	for (FInventorySlot& Item : InventoryContents)
// 	{
// 		if (Item.IsValid())
// 		{
// 			continue;
// 		}
// 		int32 ToAdd = FMath::Min(ItemData.ItemMaxStackSize, NumberToAdd);
// 		Item.SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
// 		NumberToAdd -= ToAdd;
// 		if (NumberToAdd <= 0)
// 		{
// 			UpdateWeight();
// 			return NumberToAdd;
// 		}
// 	}
// 	UpdateWeight();
// 	return NumberToAdd; // Return number not added
// }

// bool UInventoryComponent::RemoveItemFromInventory(const FName ItemID, const int32 Quantity)
// {
// 	int NumberToRemove = Quantity;
// 	TArray<int> ItemAtIndex;
// 	for (int i = InventoryContents.Num() - 1; i >= 0 ; i--)
// 	{
// 		if (InventoryContents[i].ItemID == ItemID)
// 		{
// 			ItemAtIndex.Add(i);
// 			int RemovedFromStack = FMath::Min(InventoryContents[i].Quantity, NumberToRemove);
// 			NumberToRemove -= RemovedFromStack;
// 			if (NumberToRemove <= 0)
// 			{
// 				break;
// 			}
// 		}
// 	}
// 	if (NumberToRemove > 0)
// 	{
// 		UpdateWeight();
// 		return false;
// 	}
// 	NumberToRemove = Quantity;
// 	for (const int i : ItemAtIndex)
// 	{
// 		int StackSize = InventoryContents[i].Quantity;
// 		int ToRemove = FMath::Min(NumberToRemove, StackSize);
// 		InventoryContents[i].Quantity -= ToRemove;
// 		if (!InventoryContents[i].IsValid())
// 		{
// 			InventoryContents[i].ClearItemData();
// 		}
// 		NumberToRemove -= ToRemove;
// 		if (NumberToRemove <= 0)
// 		{
// 			break;
// 		}
// 	}
// 	UpdateWeight();
// 	return true;
// }

// bool UInventoryComponent::TransferItemFromInventory(UInventoryComponent* TargetInventory, const FName& ItemID,
// 	const int32 Quantity, const int TargetIndex)
// {
// 	// Is TargetInventory valid?
// 	if (!IsValid(TargetInventory))
// 	{
// 		return false;
// 	}
// 	int TempIndex = -1;
// 	// What index is the item at
// 	for (int i = 0; i < InventoryContents.Num(); i++)
// 	{
// 		if (InventoryContents[i].ItemID == ItemID)
// 		{
// 			TempIndex = i;
// 			break;
// 		}
// 	}
// 	// Did we find the item
// 	if (TempIndex == -1)
// 	{
// 		return false;
// 	}
// 	// Copy the data
// 	FInventorySlot& TempItem = InventoryContents[TempIndex];
// 	// Can we remove the item
// 	if (!RemoveItemFromInventory(ItemID, Quantity))
// 	{
// 		return false;
// 	}
// 	// Grab the item data
// 	FItemData ItemToMove;
// 	// How many do we have to move
// 	int NumberRemaining = Quantity;
// 	
// 	NumberRemaining = TargetInventory->AddItemToSlot(ItemToMove, Quantity, TempItem.CurrentDurability, TargetIndex);
// 	// Did we move them all
// 	if (NumberRemaining > 0)
// 	{
// 		AddItemToSlot(ItemToMove, NumberRemaining, TempItem.CurrentDurability, TempIndex);
// 	}
// 	return true;
// }

// int32 UInventoryComponent::AddItemToSlot(const FItemData& ItemData, const int32 Quantity, 
//                                            const float Durability,	const int Index)
// {
// 	// No slot provided
// 	if (Index == INDEX_NONE || Index >= TotalNumberOfSlots)
// 	{
// 		// Drop into first open spot
// 		return AddItemToInventory(ItemData, Quantity, Durability);
// 	}
// 	// move to unoccupied slot
// 	int Remaining = Quantity;
// 	if (!InventoryContents[Index].IsValid())
// 	{
// 		int ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Quantity);	
// 		InventoryContents[Index].SetItemData(ItemData.ItemID, Quantity, Durability, ItemData.SingleItemWeight);
// 		Remaining -= ToAdd;
// 		if (Remaining > 0)
// 		{
// 			return AddItemToInventory(ItemData, Remaining, Durability);
// 		}
// 		UpdateWeight();
// 		return Remaining;
// 	}
// 	// move to occupied slot
// 	//		move preexisting item to empty slot or drop if not available
// 	for (FInventorySlot& Item : InventoryContents)
// 	{
// 		if (Item.IsValid())
// 		{
// 			continue;
// 		}
// 		// empty slot, place here
// 		Item = InventoryContents[Index];
// 		InventoryContents[Index].ClearItemData();
// 		int ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Quantity);
// 		InventoryContents[Index].SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
// 		Remaining -= ToAdd;
// 		if (Remaining > 0)
// 		{
// 			return AddItemToInventory(ItemData, Remaining, Durability);
// 		}
// 		UpdateWeight();
// 		return Remaining;
// 	}
// 	// Drop preexisting item
// 	DropItemFromSlotData(InventoryContents[Index]);
// 	InventoryContents[Index].ClearItemData();
// 	int ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Quantity);
// 	InventoryContents[Index].SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
// 	Remaining -= ToAdd;
// 	if (Remaining > 0)
// 	{
// 		return AddItemToInventory(ItemData, Remaining, Durability);
// 	}
// 	UpdateWeight();
// 	return Remaining;
// }

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
	int32 TempIndex = INDEX_NONE;
	int32 ToRemove = Quantity;
	int32 TotalTransferred = 0;
	for (int i = InventoryContents.Num() - 1; i >= 0; i--)
	{
		// we need to find the item given
		if (ItemID != InventoryContents[i].ItemID)
		{
			continue;
		}
		TempIndex = i;
		FInventorySlot ExtractedItem;
		if (!ExtractItemFromSlot(TempIndex, ToRemove, ExtractedItem))
		{
			return false;
		}
		int32 NotAdded = TargetInventory->Execute_AddItem(TargetInventory.GetObject(), 
			ExtractedItem.ItemID, ExtractedItem.Quantity, ExtractedItem.CurrentDurability);
		ToRemove -= (ExtractedItem.Quantity - NotAdded);
		TotalTransferred += (ExtractedItem.Quantity - NotAdded);
		// We couldnt transfer everything over, put remaining transfer amount back into original slot
		if (NotAdded > 0)
		{
			InventoryContents[TempIndex].Quantity += NotAdded;
			break;
		}
		// We finished moving all requested amount out of inventory
		if (ToRemove <= 0)
		{
			break;
		}
	}
	// We didnt find the item
	if (TempIndex == INDEX_NONE)
	{
		return false;
	}
	UpdateWeight();
	return TotalTransferred > 0;
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
	FInventorySlot ExtractedItem;
	if (!ExtractItemFromSlot(Index, Quantity, ExtractedItem))
	{
		// We couldnt take the requested item
		return false;
	}
	// Transfer to other inventory
	int32 NotAdded = TargetInventory->Execute_AddItem(TargetInventory.GetObject(), ExtractedItem.ItemID, ExtractedItem.Quantity, ExtractedItem.CurrentDurability);
	// Make sure we could transfer the requested amount
	if (NotAdded > 0)
	{
		// Put remaining back into current stack
		InventoryContents[Index].Quantity += NotAdded;
	}
	// If nothing was transferred, transfer did not complete
	if (ExtractedItem.Quantity == NotAdded)
	{
		return false;
	}
	UpdateWeight();
	return true;
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
