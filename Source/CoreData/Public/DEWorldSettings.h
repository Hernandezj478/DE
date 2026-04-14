

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "DEWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class COREDATA_API ADEWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ToolTip = "Daily temperature range"), Category = "World Settings|Temperature")
	TSoftObjectPtr<UCurveFloat> DailyTemperatureRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", ToolTip = "Annual temperature range, used as an offset to the daily temperature"), Category = "World Settings|Temperature")
	TSoftObjectPtr<UCurveFloat> AnnualTemperatureRange;
};
