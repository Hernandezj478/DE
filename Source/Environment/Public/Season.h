#pragma once

#include "CoreMinimal.h"
#include "Season.generated.h"

UENUM(BlueprintType)
enum class ESeason : uint8
{
	Spring, 
	Summer,
	Autumn,
	Winter
};
