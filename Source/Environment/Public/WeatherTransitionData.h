#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeatherType.h"
#include "WeatherState.h"
#include "WeatherTransition.h"
#include "WeatherTransitionData.generated.h"


USTRUCT(BlueprintType)
struct FWeatherTransitionList
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	TArray<FWeatherTransition> Transitions;
};


UCLASS(BlueprintType)
class ENVIRONMENT_API UWeatherTransitionData : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EWeatherType, FWeatherTransitionList> TransitionTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EWeatherType, FWeatherState> WeatherStateDefaults;
};
