#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SaveableInterface.h"
#include "DESaveGame.h"
#include "SaveGameSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleteDelegate, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleteDelegate, bool, bSuccess);

UCLASS()
class SAVESYSTEM_API USaveGameSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	void RegisterSaveable(TScriptInterface<ISaveableInterface> Saveable);
	void UnregisterSaveable(TScriptInterface<ISaveableInterface> Saveable);
	void StoreStreamingEntity(const FEntitySaveRecord& Record);
	bool TryRestoreStreamingEntity(const FName& EntityID, FEntitySaveRecord& OutRecord);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SaveAsync(const FString& SlotName);
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void LoadAsync(const FString& SlotName);

	UPROPERTY(BlueprintAssignable, Category = "Save System")
	FOnSaveCompleteDelegate OnSaveComplete;
	UPROPERTY(BlueprintAssignable, Category = "Save System")
	FOnLoadCompleteDelegate OnLoadComplete;

	UFUNCTION(BlueprintCallable, Category = "Auto Save")
	void SetAutoSaveInterval(float NewIntervalSeconds);
	UFUNCTION(BlueprintCallable, Category = "Auto Save")
	void SetAutoSaveEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Save System")
	bool HasLoadedSaveData() const
	{
		return bHasLoadedData;
	}
	UFUNCTION(BlueprintPure, Category = "Save System")
	FString GetAutoSaveSlotName() const
	{
		return AutoSaveSlotName;
	}

private:
	static const FString AutoSaveSlotName;

	FTimerHandle AutoSaveTimerHandle;
	TArray<TScriptInterface<ISaveableInterface>> RegisteredSaveables;
	TMap<FName, FEntitySaveRecord> StreamingHotStore;
	
	bool bSaveInProgress = false;
	bool bHasLoadedData = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AutoSaveIntervalSeconds = 900.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bAutoSaveEnabled = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bAutoLoadOnBeginPlay = true;
	UPROPERTY()
	UDESaveGame* pActiveSaveGame = nullptr;

	bool IsServerWorld() const;

	void OnAutoSaveTick();
	void RestartAutoSaveTimer();
	void CollectAllEntityData();
	void DistributeLoadedData();

	void TryApplyLoadedDataToSaveable(TScriptInterface<ISaveableInterface>& Saveable);

	UFUNCTION()
	void OnSaveGameComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	UFUNCTION()
	void OnLoadGameComplete(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedSave);
};
