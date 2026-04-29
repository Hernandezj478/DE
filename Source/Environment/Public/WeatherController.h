

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeatherController.generated.h"

UCLASS()
class ENVIRONMENT_API AWeatherController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeatherController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnWeatherChanged();
};
