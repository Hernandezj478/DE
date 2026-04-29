#include "WeatherController.h"
#include "MessagingSubsystem.h"
#include "EnvironmentManager.h"
#include "Logger.h"
// Sets default values
AWeatherController::AWeatherController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AWeatherController::BeginPlay()
{
	Super::BeginPlay();
	if (UMessagingSubsystem* Messenger = UMessagingSubsystem::Get())
	{
		Messenger->OnWeatherChanged.AddDynamic(this, &AWeatherController::OnWeatherChanged);
	}
}

void AWeatherController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UMessagingSubsystem* Messenger = UMessagingSubsystem::Get())
	{
		Messenger->OnWeatherChanged.RemoveDynamic(this, &AWeatherController::OnWeatherChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void AWeatherController::OnWeatherChanged()
{
	EWeatherType NewType = GetWorld()->GetSubsystem<UEnvironmentManager>()->GetCurrentWeatherType();
	switch (NewType)
	{
	case EWeatherType::Clear:
		LOG_MSG(DEBUG, "New Weather State is now Clear Skies");
		break;
	case EWeatherType::Cloudy:
		LOG_MSG(DEBUG, "New Weather State is now Cloudy");
		break;
	case EWeatherType::Overcast:
		LOG_MSG(DEBUG, "New Weather State is now Overcast");
		break;
	case EWeatherType::Rain:
		LOG_MSG(DEBUG, "New Weather State is now Rain");
		break;
	case EWeatherType::Snow:
		LOG_MSG(DEBUG, "New Weather State is now Snow");
		break;
	default:
		LOG_WARNING(LogEnvironment, "No weather state exists!");
	}
}
