#pragma once

#include "CoreMinimal.h"
#include "WeatherType.h"
#include "Season.h"
#include "WeatherTransition.generated.h"

USTRUCT(BlueprintType)
struct FWeatherTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EWeatherType ToWeatherType = EWeatherType::Clear;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Weight = 1.0f;
	
	// Optional: only valid during certain seasons or time ranges
	UPROPERTY(EditAnywhere)
	bool bSeasonRestricted = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bSeasonRestricted"))
	TArray<ESeason> AllowedSeasons;
};