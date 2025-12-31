#pragma once

#include "CoreMinimal.h"
#include "FDEStat.generated.h"


USTRUCT(BlueprintType)
struct FDEStat
{
	GENERATED_USTRUCT_BODY()
	
public:
	FDEStat() {};
	FDEStat(const float& current, const float& max, const float& tick)
	{
		CurrentValue = current;
		MaxValue = max;
		PerSecondTick = tick;
	}
	
	void TickStat(const float& DeltaTime)
	{
		CurrentValue = FMath::Clamp(CurrentValue + (PerSecondTick * DeltaTime), 0, MaxValue);
	}
	
	// Amount can be positive or negative, depending on how we want these to behave
	void Adjust(const float& Amount)
	{
		CurrentValue = FMath::Clamp(CurrentValue + Amount, 0, MaxValue);
	}
	
	float Percentile() const
	{
		return FMath::Clamp(CurrentValue / MaxValue, 0, 1.f);
	}
	
	void AdjustTick(const float& NewTick)
	{
		PerSecondTick = NewTick;
	}
	
	float GetCurrentValue() const
	{
		return CurrentValue;
	}
	float GetMaxValue() const
	{
		return MaxValue;
	}

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	float CurrentValue = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	float MaxValue = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	float PerSecondTick = 1;
};
