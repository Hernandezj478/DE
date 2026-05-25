#pragma once

#include "CoreMinimal.h"
#include "EStatTypes.generated.h"

UENUM(BlueprintType)
enum class EStatTypes : uint8
{
	ST_HEALTH		UMETA(DisplayName = "HEALTH"),
	ST_STAMINA		UMETA(DisplayName = "STAMINA"),
	ST_BLOOD		UMETA(DisplayName = "BLOOD"),
	ST_SATIATION	UMETA(DisplayName = "SATIATION"),
	ST_HYDRATION	UMETA(DisplayName = "HYDRATION"),
	ST_FATIGUE		UMETA(DisplayName = "Fatigue")
};