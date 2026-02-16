#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName ItemID = NAME_None;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Quantity = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentDurability = -1.f;	// < 0 item has no durability
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ItemWeight = 0.f;
	
	FInventorySlot() {}
	
	void SetItemData(const FName id, const int32 quantity, const float durability, const float weight)
	{
		ItemID = id;
		Quantity = quantity;
		CurrentDurability = durability;
		ItemWeight = weight;
	}
	
	FORCEINLINE void ClearItemData()
	{
		ItemID = NAME_None;
		Quantity = 0;
		CurrentDurability = -1.f;
		ItemWeight = 0.f;
	}
	
	FORCEINLINE bool IsValid() const
	{
		return !ItemID.IsNone() && Quantity > 0;
	}
};