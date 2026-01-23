// Fill out your copyright notice in the Description page of Project Settings.


#include "DEInventoryComponent.h"

#include "DEItemData.h"


UDEInventoryComponent::UDEInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

bool UDEInventoryComponent::UpdateInventorySlotCount(const int32 NewSlots)
{
	TotalNumberOfSlots = NewSlots;
	ResizeInventory();
	UpdateWeight();
	return true;
}

int32 UDEInventoryComponent::AddItemToInventory(const FDEItemData& ItemData, const int32 Quantity, const float Durability)
{
	int32 NumberToAdd = Quantity;
	for (FDEInventorySlot& Item : InventoryContents)
	{
		if (!Item.IsValid() || ItemData.ItemID != Item.ItemID || Item.Quantity >= ItemData.ItemMaxStackSize)
		{
			continue;
		}
		int32 StackSpace = ItemData.ItemMaxStackSize - Item.Quantity;
		int32 ToAdd = FMath::Min(StackSpace, NumberToAdd);
		Item.Quantity += ToAdd;
		NumberToAdd -= ToAdd;
		if (NumberToAdd <= 0)
		{
			UpdateWeight();
			return NumberToAdd;	// Return number not added
		}
	}
	for (FDEInventorySlot& Item : InventoryContents)
	{
		if (Item.IsValid())
		{
			continue;
		}
		int32 ToAdd = FMath::Min(ItemData.ItemMaxStackSize, NumberToAdd);
		Item.SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
		NumberToAdd -= ToAdd;
		if (NumberToAdd <= 0)
		{
			UpdateWeight();
			return NumberToAdd;
		}
	}
	UpdateWeight();
	return NumberToAdd; // Return number not added
}

bool UDEInventoryComponent::RemoveItemFromInventory(const FName ItemID, const int32 Quantity)
{
	int NumberToRemove = Quantity;
	TArray<int> ItemAtIndex;
	for (int i = InventoryContents.Num() - 1; i >= 0 ; i--)
	{
		if (InventoryContents[i].ItemID == ItemID)
		{
			ItemAtIndex.Add(i);
			int RemovedFromStack = FMath::Min(InventoryContents[i].Quantity, NumberToRemove);
			NumberToRemove -= RemovedFromStack;
			if (NumberToRemove <= 0)
			{
				break;
			}
		}
	}
	if (NumberToRemove > 0)
	{
		UpdateWeight();
		return false;
	}
	NumberToRemove = Quantity;
	for (const int i : ItemAtIndex)
	{
		int StackSize = InventoryContents[i].Quantity;
		int ToRemove = FMath::Min(NumberToRemove, StackSize);
		InventoryContents[i].Quantity -= ToRemove;
		if (!InventoryContents[i].IsValid())
		{
			InventoryContents[i].ClearItemData();
		}
		NumberToRemove -= ToRemove;
		if (NumberToRemove <= 0)
		{
			break;
		}
	}
	UpdateWeight();
	return true;
}

int32 UDEInventoryComponent::AddItemToSlot(const FDEItemData& ItemData, const int32 Quantity, 
	const float Durability,	const int Index)
{
	// No slot provided
	if (Index == INDEX_NONE || Index >= TotalNumberOfSlots)
	{
		// Drop into first open spot
		return AddItemToInventory(ItemData, Quantity, Durability);
	}
	// move to unoccupied slot
	int Remaining = Quantity;
	if (!InventoryContents[Index].IsValid())
	{
		int ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Quantity);	
		InventoryContents[Index].SetItemData(ItemData.ItemID, Quantity, Durability, ItemData.SingleItemWeight);
		Remaining -= ToAdd;
		if (Remaining > 0)
		{
			return AddItemToInventory(ItemData, Remaining, Durability);
		}
		UpdateWeight();
		return Remaining;
	}
	// move to occupied slot
	//		move preexisting item to empty slot or drop if not available
	for (FDEInventorySlot& Item : InventoryContents)
	{
		if (Item.IsValid())
		{
			continue;
		}
		// empty slot, place here
		Item = InventoryContents[Index];
		InventoryContents[Index].ClearItemData();
		int ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Quantity);
		InventoryContents[Index].SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
		Remaining -= ToAdd;
		if (Remaining > 0)
		{
			return AddItemToInventory(ItemData, Remaining, Durability);
		}
		UpdateWeight();
		return Remaining;
	}
	// Drop preexisting item
	DropItemFromSlotData(InventoryContents[Index]);
	InventoryContents[Index].ClearItemData();
	int ToAdd = FMath::Min(ItemData.ItemMaxStackSize, Quantity);
	InventoryContents[Index].SetItemData(ItemData.ItemID, ToAdd, Durability, ItemData.SingleItemWeight);
	Remaining -= ToAdd;
	if (Remaining > 0)
	{
		return AddItemToInventory(ItemData, Remaining, Durability);
	}
	UpdateWeight();
	return Remaining;
}

void UDEInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ResizeInventory();
	UpdateWeight();
}

void UDEInventoryComponent::ResizeInventory()
{
	if (InventoryContents.Num() == TotalNumberOfSlots)
	{
		return;
	}
	if (InventoryContents.Num() < TotalNumberOfSlots)
	{
		while (InventoryContents.Num() < TotalNumberOfSlots)
		{
			InventoryContents.Add(FDEInventorySlot());
		}
		UpdateWeight();
		return;
	}
	while (InventoryContents.Num() > TotalNumberOfSlots)
	{
		FDEInventorySlot DroppedItem = InventoryContents.Pop();
		if (DroppedItem.IsValid())
		{
			// TODO: Drop extra items in the world
			DropItemFromSlotData(DroppedItem);
		}
	}
	UpdateWeight();
}

void UDEInventoryComponent::UpdateWeight()
{
	// TODO: create event on bus to signal if weight is over max carry capacity.
	float TempWeight = 0.f;
	for (const FDEInventorySlot& Item : InventoryContents)
	{
		if (!Item.IsValid())
		{
			continue;
		}
		TempWeight += (Item.Quantity * Item.ItemWeight);
	}
	CurrentCarryWeight = TempWeight;
}

void UDEInventoryComponent::DropItemToWorldAtIndex(const int Index)
{
	//TODO: call world spawn manager to spawn dropped item
}

void UDEInventoryComponent::DropItemFromSlotData(const FDEInventorySlot& SlotData)
{
	
}

