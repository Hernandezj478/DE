#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveableInterface.h"
#include "DESaveGame.generated.h"


UCLASS()
class SAVESYSTEM_API UDESaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame)
	TMap<FName, FEntitySaveRecord> EntityData;

	UPROPERTY(SaveGame)
	TMap<FName, FEntitySaveRecord> StreamingEntityData;

	UPROPERTY(SaveGame)
	FString SlotName;

	UPROPERTY(SaveGame)
	FDateTime RealWorldSaveTime;

	UPROPERTY(SaveGame)
	int32 SaveVersion = 1;

	UPROPERTY(SaveGame)
	FGuid PlayerGUID;
};
