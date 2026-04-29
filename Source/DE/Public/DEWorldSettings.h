#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "DEWorldSettings.generated.h"

UCLASS()
class DE_API ADEWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ToolTip = "Daily temperature range"), Category = "World Settings|Temperature")
	TSoftObjectPtr<UCurveFloat> DailyTemperatureRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ToolTip = "Annual temperature range, used as an offset to the daily temperature"), Category = "World Settings|Temperature")
	TSoftObjectPtr<UCurveFloat> AnnualTemperatureRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ToolTip = "Weather data for the current map"), Category = "World Settings|Weather")
	TSoftObjectPtr<UDataAsset> WorldWeatherData;
};
