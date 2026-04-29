#pragma once

#include "CoreMinimal.h"
#include "FTimeData.generated.h"

USTRUCT(BlueprintType)
struct FTimeData
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int DayOfYear = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int Year = 2026;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int Month = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int DayOfMonth = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int Hour = 11;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame)
	int Minute = 0;
	
	float GetTimeOfDay() const {return (Hour * 60) + Minute;}
	
	void SetValues(int InDayOfYear, int InYear, int InMonth, int InDayOfMonth, int InHour, int InMinute)
	{
		DayOfYear = InDayOfYear;
		Year = InYear;
		Month = InMonth;
		DayOfMonth = InDayOfMonth;
		Hour = InHour;
		Minute = InMinute;
	}
	
	FString GetTimeString() const
	{
		FString TimeString;
		TimeString += FString::FromInt(DayOfYear);
		TimeString += "_";
		TimeString += FString::FromInt(DayOfMonth);
		TimeString += "-";
		TimeString += FString::FromInt(Month);
		TimeString += "-";
		TimeString += FString::FromInt(Year);
		TimeString += "_";
		TimeString += FString::FromInt(Hour);
		TimeString += ":";
		TimeString += FString::FromInt(Minute);
		return TimeString;
	}
};