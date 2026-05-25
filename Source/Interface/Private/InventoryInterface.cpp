#include "InventoryInterface.h"

int32 IInventoryInterface::AddItem_Implementation(const FName& ItemID, const int32& Quantity, const float& Durability)
{
	return int32();
}

bool IInventoryInterface::RemoveItem_Implementation(const FName& ItemID, const int32& Quantity)
{
	return false;
}

bool IInventoryInterface::TransferItem_Implementation(const FName& ItemID, const int32& Quantity, const TScriptInterface<IInventoryInterface>& TargetInventory)
{
	return false;
}

int32 IInventoryInterface::AddItemToSlot_Implementation(const int32& Index, const FName& ItemID, const int32& Quantity, const float& Durability)
{
	return int32();
}

bool IInventoryInterface::RemoveItemFromSlot_Implementation(const int32& Index, const int32& Quantity)
{
	return false;
}

bool IInventoryInterface::TransferItemFromSlot_Implementation(const int32& Index, const int32& Quantity, const TScriptInterface<IInventoryInterface>& TargetInventory)
{
	return false;
}

int32 IInventoryInterface::GetAvailableSpace_Implementation(const FName& ItemID, const int32& Quantity) const
{
	return int32();
}
