#pragma once

#include "CoreMinimal.h"
#include "FStat.generated.h"


USTRUCT(BlueprintType)
struct FStat
{
	GENERATED_USTRUCT_BODY()
	
public:
	FStat() {};
	FStat(const float& current, const float& max, const float& tick)
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
	
	void SetMaxValue(float NewMax)
	{
		CurrentValue = NewMax * (CurrentValue / MaxValue);	
		MaxValue = NewMax;
	}
	void SetCurrentValue(float NewValue)
	{
		CurrentValue = NewValue;
	}
	float GetCurrentValue() const
	{
		return CurrentValue;
	}
	float GetMaxValue() const
	{
		return MaxValue;
	}
	float GetTickRate() const
	{
		return PerSecondTick;
	}
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		// Serialize current and max
		Ar << CurrentValue;
		Ar << MaxValue;
		bOutSuccess = true;
		return true;
	}
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	float CurrentValue = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	float MaxValue = 100;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = true))
	float PerSecondTick = 1;
};

template<>
struct TStructOpsTypeTraits<FStat> : public TStructOpsTypeTraitsBase2<FStat>
{
	enum { WithNetSerializer = true };
};