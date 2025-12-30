#pragma once

#include "CoreMinimal.h"
#include "EDEStatType.generated.h"

UENUM(BlueprintType)
enum class EDEStatType : uint8
{
	ST_HEALTH		UMETA(DisplayName = "HEALTH"),
	ST_STAMINA		UMETA(DisplayName = "STAMINA"),
	ST_SATIATION	UMETA(DisplayName = "SATIATION"),
	ST_HYDRATION	UMETA(DisplayName = "HYDRATION")
	// Blood
	// Calories
	// Protein
	// Fats
	// Carbs
	// Vitamin
};