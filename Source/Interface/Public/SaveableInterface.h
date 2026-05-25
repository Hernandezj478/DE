

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableInterface.generated.h"


USTRUCT(BlueprintType)
struct FSaveComponentData
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadWrite)
	TArray<uint8> ByteData;
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadWrite)
	UClass* ComponentClass = nullptr;
	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadWrite)
	TArray<FString> RawData;
};

USTRUCT(BlueprintType)
struct FEntitySaveRecord
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(SaveGame)
	FName EntityID = NAME_None;
	UPROPERTY(SaveGame)
	FTransform ActorTransform = FTransform();
	UPROPERTY(SaveGame)
	TArray<uint8> CustomData;
	UPROPERTY(SaveGame)
	TArray<FSaveComponentData> ComponentData;
	UPROPERTY(SaveGame)
	TArray<FString> RawData;
	UPROPERTY(SaveGame)
	bool bWasRuntimeSpawned = false;
	UPROPERTY(SaveGame)
	UClass* ActorClass = nullptr;
};


UINTERFACE(MinimalAPI)
class USaveableInterface : public UInterface
{
	GENERATED_BODY()
};

class INTERFACE_API ISaveableInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	FName GetSaveID() const;
	virtual FName GetSaveID_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	bool CollectSaveData(FEntitySaveRecord& OutRecord) const;
	virtual bool CollectSaveData_Implementation(FEntitySaveRecord& OutRecord) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void ApplySaveData(const FEntitySaveRecord& Record);
	virtual void ApplySaveData_Implementation(const FEntitySaveRecord& Record);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void OnPreSave();
	virtual void OnPreSave_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void OnPostLoad();
	virtual void OnPostLoad_Implementation();
};
