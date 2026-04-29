#pragma once

#include "CoreMinimal.h"
#include "WeatherType.h"
#include "WeatherState.generated.h"	

USTRUCT(BlueprintType)
struct FWeatherState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EWeatherType WeatherType = EWeatherType::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CloudCoverage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PercipitationIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WindSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector WindDirection = FVector::ForwardVector;

	// Temperature modifier layered on top of existing ambient temperature
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TemeratureModifier = 0.0f;

	// How long this state should persist before re-evaluating (in game-hours)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxDuration = 4.0f;
};