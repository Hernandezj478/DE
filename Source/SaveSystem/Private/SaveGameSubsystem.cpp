#include "SaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Logger.h"

const FString USaveGameSubsystem::AutoSaveSlotName = TEXT("AutoSave");

void USaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	pActiveSaveGame = Cast<UDESaveGame>(UGameplayStatics::CreateSaveGameObject(UDESaveGame::StaticClass()));
}

void USaveGameSubsystem::Deinitialize()
{
	if (GetWorld() && AutoSaveTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}
	RegisteredSaveables.Empty();
	Super::Deinitialize();
}

void USaveGameSubsystem::OnWorldBeginPlay(UWorld & InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	// TODO: Check if its a playable map

	// Subsystem exists on both server and client worlds, only server world drives save/load
	// clients resove state via replication
	if (!IsServerWorld())
	{
		return;
	}
	if (bAutoSaveEnabled)
	{
		RestartAutoSaveTimer();
	}
	if (bAutoLoadOnBeginPlay && UGameplayStatics::DoesSaveGameExist(AutoSaveSlotName, 0))
	{
		LoadAsync(AutoSaveSlotName);
	}
}

void USaveGameSubsystem::RegisterSaveable(TScriptInterface<ISaveableInterface> Saveable)
{
	if (!IsServerWorld())
	{
		return;
	}
	if (!Saveable)
	{
		LOG_ERROR(LogSaveGameSubsystem, "No valid saveable interface found");
		return;
	}
	RegisteredSaveables.AddUnique(Saveable);
	if (bHasLoadedData)
	{
		TryApplyLoadedDataToSaveable(Saveable);
	}
}

void USaveGameSubsystem::UnregisterSaveable(TScriptInterface<ISaveableInterface> Saveable)
{
	if (!IsServerWorld())
	{
		return;
	}
	if (!Saveable)
	{
		LOG_ERROR(LogSaveGameSubsystem, "No valid saveable interface found");
		return;
	}
	RegisteredSaveables.Remove(Saveable);
}

void USaveGameSubsystem::StoreStreamingEntity(const FEntitySaveRecord & Record)
{
	if (Record.EntityID == NAME_None)
	{
		LOG_WARNING(LogSaveGameSubsystem, "EntityID is NONE");
		return;
	}
	StreamingHotStore.Add(Record.EntityID, Record);
}

bool USaveGameSubsystem::TryRestoreStreamingEntity(const FName & EntityID, FEntitySaveRecord & OutRecord)
{
	if (FEntitySaveRecord* pRecord = StreamingHotStore.Find(EntityID))
	{
		OutRecord = *pRecord;
		return true;
	}
	return false;
}

void USaveGameSubsystem::SaveAsync(const FString& SlotName)
{
	if (!IsServerWorld())
	{
		LOG_WARNING(LogSaveGameSubsystem, "Client attempted save - ignoring save request");
		return;
	}
	if (bSaveInProgress)
	{
		LOG_WARNING(LogSaveGameSubsystem, "Save already in progress");
		return;
	}
	if (!IsValid(pActiveSaveGame))
	{
		LOG_ERROR(LogSaveGameSubsystem, "Active save game invalid");
		return;
	}
	bSaveInProgress = true;
	pActiveSaveGame->SlotName = SlotName;
	pActiveSaveGame->RealWorldSaveTime = FDateTime::Now();

	CollectAllEntityData();

	for (const auto& Pair : StreamingHotStore)
	{
		pActiveSaveGame->StreamingEntityData.Add(Pair.Key, Pair.Value);
	}
	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this, &USaveGameSubsystem::OnSaveGameComplete);
	//TODO: UserIndex 0 - listen server model has single host server, revisit for dedicated servers
	UGameplayStatics::AsyncSaveGameToSlot(pActiveSaveGame, SlotName, 0, SaveDelegate);
}

void USaveGameSubsystem::LoadAsync(const FString & SlotName)
{
	if (!IsServerWorld())
	{
		LOG_WARNING(LogSaveGameSubsystem, "Client attemped load - ingorning load request");
		return;
	}
	if (bSaveInProgress)
	{
		LOG_WARNING(LogSaveGameSubsystem, "Operation already in progress");
		return;
	}
	// TODO: UserIndex 0 - listen server model has single host server, revisit for dedicated servers
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		LOG_WARNING(LogSaveGameSubsystem, "No save found in slot %s", *SlotName);
		return;
	}
	bSaveInProgress = true;
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(this, &USaveGameSubsystem::OnLoadGameComplete);
	//TODO: UserIndex 0 - listen server model has single host server, revisit for dedicated servers
	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, 0, LoadDelegate);
}

void USaveGameSubsystem::SetAutoSaveInterval(float NewIntervalSeconds)
{
	AutoSaveIntervalSeconds = FMath::Max(60.f, NewIntervalSeconds);
	if (bAutoSaveEnabled && IsServerWorld())
	{
		RestartAutoSaveTimer();
	}
}

void USaveGameSubsystem::SetAutoSaveEnabled(bool bEnabled)
{
	if (!IsServerWorld())
	{
		return;
	}

	bAutoSaveEnabled = bEnabled;
	if (bAutoSaveEnabled)
	{
		RestartAutoSaveTimer();
	}
	else if(GetWorld())
	{	
		GetWorld()->GetTimerManager().ClearTimer(AutoSaveTimerHandle);	
	}
}

bool USaveGameSubsystem::IsServerWorld() const
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Client;
}

void USaveGameSubsystem::OnAutoSaveTick()
{
	LOG_DEBUG(LogSaveGameSubsystem, "Auto Save Triggered");
	SaveAsync(AutoSaveSlotName);
}

void USaveGameSubsystem::RestartAutoSaveTimer()
{
	if (!GetWorld())
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(AutoSaveTimerHandle, this, &USaveGameSubsystem::OnAutoSaveTick, 
		AutoSaveIntervalSeconds, true);
}

void USaveGameSubsystem::CollectAllEntityData()
{
	if (!IsServerWorld() || !IsValid(pActiveSaveGame))
	{
		return;
	}
	pActiveSaveGame->EntityData.Empty();
	for (TScriptInterface<ISaveableInterface>& Saveable : RegisteredSaveables)
	{
		if (!Saveable)
		{
			continue;
		}
		ISaveableInterface::Execute_OnPreSave(Saveable.GetObject());
		FEntitySaveRecord Record;
		if (ISaveableInterface::Execute_CollectSaveData(Saveable.GetObject(), Record))
		{
			if (Record.EntityID == NAME_None)
			{
				LOG_WARNING(LogSaveGameSubsystem, "Entity returned NAME_None, skipping");
				continue;
			}
			pActiveSaveGame->EntityData.Add(Record.EntityID, Record);
		}
	}
}

void USaveGameSubsystem::DistributeLoadedData()
{
	if (!IsServerWorld() || !IsValid(pActiveSaveGame))
	{
		LOG_ERROR(LogSaveGameSubsystem, "Invalid state");
		return;
	}
	StreamingHotStore = pActiveSaveGame->StreamingEntityData;
	// Iterate over save for spawned actors
	for (TScriptInterface<ISaveableInterface>& Saveable : RegisteredSaveables)
	{
		if (!Saveable)
		{
			continue;
		}
		TryApplyLoadedDataToSaveable(Saveable);
	}
	for (const auto& Pair : pActiveSaveGame->EntityData)
	{
		const FEntitySaveRecord& Record = Pair.Value;
		if (!Record.bWasRuntimeSpawned || !IsValid(Record.ActorClass))
		{
			continue;
		}
		bool bAlreadyPresent = RegisteredSaveables.ContainsByPredicate(
			[&](const TScriptInterface<ISaveableInterface>& Save) -> bool
			{
				return Save && ISaveableInterface::Execute_GetSaveID(Save.GetObject()) == Record.EntityID;
			});
		if (bAlreadyPresent)
		{
			continue;
		}
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* Spawned = GetWorld()->SpawnActor<AActor>(Record.ActorClass, Record.ActorTransform, Params);
		if (!IsValid(Spawned))
		{
			LOG_ERROR(LogSaveGameSubsystem, "Failed to spawn actor of class %s", *Record.ActorClass->GetName());
			continue;
		}
		if (Spawned->Implements<USaveableInterface>())
		{
			ISaveableInterface::Execute_ApplySaveData(Spawned, Record);
			ISaveableInterface::Execute_OnPostLoad(Spawned);
		}
	}
}

void USaveGameSubsystem::TryApplyLoadedDataToSaveable(TScriptInterface<ISaveableInterface>& Saveable)
{
	if (!IsValid(pActiveSaveGame) || !Saveable)
	{
		return;
	}
	const FName SaveID = ISaveableInterface::Execute_GetSaveID(Saveable.GetObject());
	if (FEntitySaveRecord* pRecord = pActiveSaveGame->EntityData.Find(SaveID))
	{
		ISaveableInterface::Execute_ApplySaveData(Saveable.GetObject(), *pRecord);
		ISaveableInterface::Execute_OnPostLoad(Saveable.GetObject());
	}
}

void USaveGameSubsystem::OnSaveGameComplete(const FString & SlotName, const int32 UserIndex, bool bSuccess)
{
	bSaveInProgress = false;
	if (bSuccess)
	{
		LOG_DEBUG(LogSaveGameSubsystem, "Saved: %s", *SlotName);
	}
	else
	{
		LOG_ERROR(LogSaveGameSubsystem, "Save FAILED: %s", *SlotName);
	}
	OnSaveComplete.Broadcast(bSuccess);
}

void USaveGameSubsystem::OnLoadGameComplete(const FString & SlotName, const int32 UserIndex, USaveGame * LoadedSave)
{
	bSaveInProgress = false;
	UDESaveGame* LoadedDESave = Cast<UDESaveGame>(LoadedSave);
	if (!IsValid(LoadedDESave))
	{
		LOG_ERROR(LogSaveGameSubsystem, "Cast failed for slot %s", *SlotName);
		OnLoadComplete.Broadcast(false);
		return;
	}
	pActiveSaveGame = LoadedDESave;
	DistributeLoadedData();
	LOG_DEBUG(LogSaveGameSubsystem, "Loaded: %s", *SlotName);
	OnLoadComplete.Broadcast(true);
}
