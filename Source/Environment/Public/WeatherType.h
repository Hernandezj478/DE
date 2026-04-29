#pragma once

#include "CoreMinimal.h"
#include "WeatherType.generated.h"

UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Clear,
	Cloudy,
	Overcast,
	Rain,
	Snow
};

static bool FindWeatherType(EWeatherType InWeatherType)
{
	switch (InWeatherType)
	{
		case EWeatherType::Clear:
		case EWeatherType::Cloudy:
		case EWeatherType::Overcast:
		case EWeatherType::Rain:
		case EWeatherType::Snow:
			return true;
		default:
			return false;
	}
}