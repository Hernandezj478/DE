#pragma once

#include "CoreMinimal.h"
#include "ECreatureType.generated.h"

UENUM(BlueprintType)
enum class ECreatureType : uint8
{
	CT_DEFULT			UMETA(DisplayName = "Default"),
	CT_HUMAN			UMETA(DisplayName = "Human"),
	CT_SMALLANIMAL		UMETA(DisplayName = "SmallAnimal"),
	CT_SMALLCARNIVORE	UMETA(DisplayName = "SmallCarnivore"),
	CT_MEDIUMANUMAL		UMETA(DisplayName = "MediumAnimal"),
	CT_MEDIUMCARNIVORE	UMETA(DisplayName = "MediumCarnivore"),
	CT_LARGEANIMAL		UMETA(DisplayName = "LargeAnimal"),
	CT_LARGECARNIVORE	UMETA(DisplayName = "LargeCarnivore"),
	CT_MONSTER			UMETA(DisplayName = "Monster")
};