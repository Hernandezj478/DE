#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DEItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemMaxStackSize = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDurability = -1.0f; //0-1 as percentile, <0 not used
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseStaticMesh = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemDescription = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SingleItemWeight = 0.f;
	
	FORCEINLINE bool IsValid() const
	{
		return !ItemID.IsNone() && ItemMaxStackSize > 0;
	}
	
	FORCEINLINE void Clear()
	{
		ItemID = NAME_None;
		ItemMaxStackSize = 0;
		MaxDurability = 0.0f;
		Mesh = nullptr;
		SkeletalMesh = nullptr;
		ItemName = FText::GetEmpty();
		ItemDescription = FText::GetEmpty();
		SingleItemWeight = 0.f;
	}
};
