#include "VoxelWorldActor.h"
#include "MarchingCubeMesher.h"
#include "HeightmapProcessor.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Texture2D.h"
#include "Async/Async.h"
#include "Logger.h"

static FORCEINLINE float Hash(int32 X, int32 Y, int32 Z, int32 Seed)
{
	int32 N = X + Y * 57 + Z * 113 + Seed * 131;
	N = (N << 13) ^ N;
	const int32 M = (N * (N * N * 15731 + 789221) + 1376312589) & 0x7fffffff;
	return 1.0f - (M / 1073741824.0f);
}

static FORCEINLINE float Fade(float T)
{
	return T * T * T * (T * (T * 6.f - 15.f) + 10.f);
}

static float ValueNoise(float X, float Y, float Z, int32 Seed)
{
	const int32 X0 = FMath::FloorToInt(X);
	const int32 Y0 = FMath::FloorToInt(Y);
	const int32 Z0 = FMath::FloorToInt(Z);
	const int32 X1 = X0 + 1;
	const int32 Y1 = Y0 + 1;
	const int32 Z1 = Z0 + 1;

	// Fractional part
	const float FX = X - X0;
	const float FY = Y - Y0;
	const float FZ = Z - Z0;

	// Fade curve for smoother interpolation
	const float UX = Fade(FX);
	const float UY = Fade(FY);
	const float UZ = Fade(FZ);

	// Hash the 8 corners
	const float V000 = Hash(X0, Y0, Z0, Seed);
	const float V100 = Hash(X1, Y0, Z0, Seed);
	const float V010 = Hash(X0, Y1, Z0, Seed);
	const float V110 = Hash(X1, Y1, Z0, Seed);
	const float V001 = Hash(X0, Y0, Z1, Seed);
	const float V101 = Hash(X1, Y0, Z1, Seed);
	const float V011 = Hash(X0, Y1, Z1, Seed);
	const float V111 = Hash(X1, Y1, Z1, Seed);

	return FMath::Lerp(
		FMath::Lerp(FMath::Lerp(V000, V100, UX), FMath::Lerp(V010, V110, UX), UY),
		FMath::Lerp(FMath::Lerp(V001, V101, UX), FMath::Lerp(V011, V111, UX), UY),
		UZ
	);
}

static float FBM(float X, float Y, float Z, int32 Seed, int32 Octaves = 4,
	float Frequency = 1.0f, float Amplitude = 1.0f,
	float Lacunarity = 2.0f, float Persistence = 0.5f)
{
	float Value = 0.0f;
	float Amp = Amplitude;
	float Freq = Frequency;
	float MaxValue = 0.0f;

	for (int32 i = 0; i < Octaves; i++)
	{
		Value += ValueNoise(X * Freq, Y * Freq, Z * Freq, Seed + i) * Amp;
		MaxValue += Amp;
		Amp *= Persistence;
		Freq *= Lacunarity;
	}
	// Avoid divide by 0
	if (MaxValue <= SMALL_NUMBER) MaxValue = 1.f;
	// Normalize to [-1, 1]
	return Value / MaxValue;
}

AVoxelWorldActor::AVoxelWorldActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AVoxelWorldActor::BeginPlay()
{
	Super::BeginPlay();
	if(!bTerrainBuilt)
	{
		RebuildTerrain();
	}
}

void AVoxelWorldActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UWorld* World = GetWorld();
	if (!World) return;
	if (World->WorldType != EWorldType::Editor && !World->IsGameWorld())
	{
		return;
	}
	RebuildTerrain();
}

void AVoxelWorldActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RebuildTerrain();
}

void AVoxelWorldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ProcessPendingMeshQueue();
	ProcessUploadQueue();
}

void AVoxelWorldActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVoxelWorldActor, ReplicatedSeed);
	DOREPLIFETIME(AVoxelWorldActor, ModificationLog);
}

void AVoxelWorldActor::RebuildTerrain()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (World->WorldType != EWorldType::Editor && !World->IsGameWorld())
	{
		return;
	}
	{
		FScopeLock Lock(&UploadQueueMutex);
		UploadQueue.Empty();
	}
	PendingMeshQueue.Empty();
	if (TerrainSource == EVoxelTerrainSource::Heightmap && HeightmapTexture)
	{
		if (!HeightmapProcessor)
		{
			HeightmapProcessor = NewObject<UHeightmapProcessor>(this);

		}
		HeightmapProcessor->MinHeightVoxels = HeightmapMinVoxels;
		HeightmapProcessor->MaxHeightVoxels = HeightmapMaxVoxels;

		HeightmapProcessor->LoadImage(HeightmapTexture);

		if (!HeightmapProcessor->IsLoaded())
		{
			LOG_ERROR(LogVoxelEngine, "Failed to load heightmap texture.");
			HeightmapProcessor = nullptr;
		}
		else
		{
			LOG_DEBUG(LogVoxelEngine, "Heightmap Loaded - Pixels: %dx%d, Height Range: %.1f-%.1f",
				HeightmapProcessor->GetImageWidth(), HeightmapProcessor->GetImageHeight(),
				HeightmapMinVoxels, HeightmapMaxVoxels);
		}
	}
	else if (TerrainSource != EVoxelTerrainSource::Heightmap)
	{
		HeightmapProcessor = nullptr;
	}
	DestroyChunkGrid();
	CreateChunkGrid();
	LastReplayedIndex = 0;
}

void AVoxelWorldActor::DigSphere(FVector WorldCenter, float Radius, float Strength)
{
	const FVoxelModification Mod(EVoxelOpType::DigSphere, WorldCenter, Radius, Strength);

	TSet<FChunkCoord> DirtyChunks = ApplyDigSphere(WorldCenter, Radius, Strength);
	RemeshDirtyChunks(DirtyChunks);

	ModificationLog.Add(Mod);

	if (ModificationLog.Num() > MaxModificationLogSize)
	{
		LOG_WARNING(LogVoxelEngine, "Modificationlog exceeded %d enteries. Consider compactions", MaxModificationLogSize);
	}
	MulticastApplyDig(WorldCenter, Radius, Strength);
}

void AVoxelWorldActor::AddSphere(FVector WorldCenter, float Radius, float Strength)
{
	const FVoxelModification Mod(EVoxelOpType::AddSphere, WorldCenter, Radius, Strength);
	TSet<FChunkCoord> DirtyChunks = ApplyAddSphere(WorldCenter, Radius, Strength);
	RemeshDirtyChunks(DirtyChunks);
	if (ModificationLog.Num() > MaxModificationLogSize)
	{
		LOG_WARNING(LogVoxelEngine, "ModificationLog exceeded %d entries. Consider compation", MaxModificationLogSize);
	}
	ModificationLog.Add(Mod);
	MulticastApplyAdd(WorldCenter, Radius, Strength);
}

void AVoxelWorldActor::MulticastApplyDig_Implementation(FVector WorldCenter, float Radius, float Strength)
{
	if (HasAuthority()) return;
	TSet<FChunkCoord> DirtyChunks = ApplyDigSphere(WorldCenter, Radius, Strength);
	RemeshDirtyChunks(DirtyChunks);
	++LastReplayedIndex;
}

void AVoxelWorldActor::MulticastApplyAdd_Implementation(FVector WorldCenter, float Radius, float Strength)
{
	if (HasAuthority()) return;
	TSet<FChunkCoord> DirtyChunks = ApplyAddSphere(WorldCenter, Radius, Strength);
	RemeshDirtyChunks(DirtyChunks);
	++LastReplayedIndex;
}

void AVoxelWorldActor::CreateChunkGrid()
{
	const FChunkCoord ActorChunkCoord = FChunkCoord::FromWorldPosition(GetActorLocation());
	for (int32 CZ = -ViewDistance; CZ <= ViewDistance; CZ++)
	{
		for (int32 CY = -ViewDistance; CY <= ViewDistance; CY++)
		{
			for (int32 CX = -ViewDistance; CX <= ViewDistance; CX++)
			{
				const FChunkCoord Coord(
					ActorChunkCoord.X + CX, 
					ActorChunkCoord.Y + CY, 
					ActorChunkCoord.Z + CZ + SurfaceChunkZ);
				UVoxelChunkComponent* Chunk = CreateChunk(Coord);
				DensityTaskAsync(Chunk);
			}
		}
	}
	bTerrainBuilt = true;
}

void AVoxelWorldActor::DestroyChunkGrid()
{
	for (TPair<FChunkCoord, UVoxelChunkComponent*>& Pair : Chunks)
	{
		UVoxelChunkComponent* Chunk = Pair.Value;
		if (!Chunk)
		{
			continue;
		}
		if (UProceduralMeshComponent* Mesh = Chunk->GetMeshComponent())
		{
			Mesh->DestroyComponent();
		}
		Chunk->DestroyComponent();
	}
	Chunks.Empty();
	bTerrainBuilt = false;
}

UVoxelChunkComponent* AVoxelWorldActor::CreateChunk(const FChunkCoord& Coord)
{
	const FName CompName = FName(*FString::Printf(
		TEXT("Chunk_%d_%d_%d"), Coord.X, Coord.Y, Coord.Z));
	UVoxelChunkComponent* NewChunk = NewObject<UVoxelChunkComponent>(this, CompName);
	NewChunk->ChunkCoord = Coord;
	NewChunk->RegisterComponent();
	NewChunk->SetState(EChunkState::Uninitialized);

	Chunks.Add(Coord, NewChunk);
	return NewChunk;
}

void AVoxelWorldActor::DensityTaskAsync(UVoxelChunkComponent* Chunk)
{
	Chunk->SetState(EChunkState::GeneratingDensity);
	const EVoxelTerrainSource CapturedSource = TerrainSource;
	const FVector ChunkOrigin = Chunk->GetChunkWorldOrigin();
	const FVector CapturedActorLocation = GetActorLocation();
	const float CapturedFreq = NoiseFrequency;
	const float CapturedAmp = NoiseAmplitude;
	const float CapturedSurface = SurfaceLevel;
	const int32 CapturedSeed = NoiseSeed;
	const float CapturedCaveFreq = CaveFrequency;
	const float CapturedCaveAmp = CaveAmplitude;
	const bool CapturedHMCaves = bHeightmapAddCaves;
	const float CapturedHMCaveFreq = HeightmapCaveFrequency;
	const float CapturedHMCaveAmp = HeightmapCaveAmplitude;

	UHeightmapProcessor* CapturedHM = HeightmapProcessor;
	TWeakObjectPtr<UVoxelChunkComponent> WeakChunk(Chunk);
	TWeakObjectPtr<AVoxelWorldActor> WeakSelf(this);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=]()
	{
		UVoxelChunkComponent* C = WeakChunk.Get();
		if (!C)
		{
			return;
		}
		if (CapturedSource == EVoxelTerrainSource::Heightmap && CapturedHM && CapturedHM->IsLoaded())
		{
			
			const float HalfW = CapturedHM->GetImageWidth() * VOXEL_SIZE * 0.5f;
			const float HalfH = CapturedHM->GetImageHeight() * VOXEL_SIZE * 0.5f;
			const float OriginX = -HalfW;
			const float OriginY = -HalfH;
			for (int32 LY = 0; LY < CHUNK_SAMPLE_SIZE; LY++)
			{
				for (int32 LX = 0; LX < CHUNK_SAMPLE_SIZE; LX++)
				{
					const float WX = ChunkOrigin.X + LX * VOXEL_SIZE;
					const float WY = ChunkOrigin.Y + LY * VOXEL_SIZE;

					const float TerrainH = CapturedHM->GetHeightAtWorld(WX, WY, OriginX, OriginY, VOXEL_SIZE);
					for (int32 LZ = 0; LZ < CHUNK_SAMPLE_SIZE; LZ++)
					{
						const float WZ = ChunkOrigin.Z + LZ * VOXEL_SIZE;
						const float RelativeZ = WZ - CapturedActorLocation.Z;
						const float SurfDensity = (RelativeZ / VOXEL_SIZE) - TerrainH;
						float CaveDensity = 0.f;
						if (CapturedHMCaves && SurfDensity < -4.f)
						{
							const float CN = FBM(
								WX * CapturedHMCaveFreq,
								WY * CapturedHMCaveFreq,
								WZ * CapturedHMCaveFreq,
								CapturedSeed + 999, 3, 1.f, 1.f, 2.f, 0.6f);
							const float Blend = FMath::SmoothStep(-4.f, -8.f, SurfDensity);
							CaveDensity = CN * CapturedHMCaveAmp * Blend;
						}
						C->SetDensity(LX, LY, LZ, SurfDensity + CaveDensity);
					}
				}
			}
		}
		else if (CapturedSource == EVoxelTerrainSource::ProceduralNoise)
		{
			for (int32 LY = 0; LY < CHUNK_SAMPLE_SIZE; LY++)
			{
				for (int32 LX = 0; LX < CHUNK_SAMPLE_SIZE; LX++)
				{
					const float WX = ChunkOrigin.X + LX * VOXEL_SIZE;
					const float WY = ChunkOrigin.Y + LY * VOXEL_SIZE;

					const float SurfNoise = FBM(
						WX * CapturedFreq,
						WY * CapturedFreq,
						0.f,
						CapturedSeed);

					const float SurfaceH = CapturedSurface + SurfNoise * CapturedAmp;
					for (int32 LZ = 0; LZ < CHUNK_SAMPLE_SIZE; LZ++)
					{
						const float WZ = ChunkOrigin.Z + LZ * VOXEL_SIZE;
						const float SurfDensity = (WZ / VOXEL_SIZE) - SurfaceH;
						float CaveDensity = 0.f;
						if (SurfDensity < -4.f)
						{
							const float CN = FBM(
								WX * CapturedCaveFreq,
								WY * CapturedCaveFreq,
								WZ * CapturedCaveFreq,
								CapturedSeed + 999, 3, 1.f, 1.f, 2.f, 0.6f);
							const float Blend = FMath::SmoothStep(-4.f, -8.f, SurfDensity);
							CaveDensity = CN * CapturedCaveAmp * Blend;
						}
						C->SetDensity(LX, LY, LZ, SurfDensity + CaveDensity);
					}
				}
			}
		}
		AsyncTask(ENamedThreads::GameThread, [WeakChunk, WeakSelf]()
		{
			UVoxelChunkComponent* Chunk = WeakChunk.Get();
			AVoxelWorldActor* Self = WeakSelf.Get();
			if (!Chunk || !Self)
			{
				return;
			}
			Chunk->SetState(EChunkState::PendingMesh);
			Self->PendingMeshQueue.Add(Chunk);
		});
	});
}

void AVoxelWorldActor::MeshTaskAsync(UVoxelChunkComponent* Chunk)
{
	Chunk->SetState(EChunkState::GeneratingMesh);
	++ActiveMeshTasks;

	TWeakObjectPtr<UVoxelChunkComponent> WeakChunk(Chunk);
	TWeakObjectPtr<AVoxelWorldActor> WeakSelf(this);
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakChunk, WeakSelf]()
	{
		UVoxelChunkComponent* C = WeakChunk.Get();
		AVoxelWorldActor* S = WeakSelf.Get();
		if (!C || !S)
		{
			if (S)
			{
				--S->ActiveMeshTasks;
			}
			return;
		}
		FChunkMeshData NewMeshData;
		FMarchingCubeMesher::MeshChunk(*C, NewMeshData);
		{
			FScopeLock Lock(&C->MeshDataMutex);
			C->PendingMeshData = MoveTemp(NewMeshData);
		}
		C->SetState(EChunkState::PendingUpload);
		--S->ActiveMeshTasks;
		AsyncTask(ENamedThreads::GameThread, [WeakChunk, WeakSelf]() 
		{
			UVoxelChunkComponent* Chunk = WeakChunk.Get();
			AVoxelWorldActor* Self = WeakSelf.Get();
			if (!Chunk || !Self)
			{
				return;
			}
			FScopeLock Lock(&Self->UploadQueueMutex);
			Self->UploadQueue.Add(Chunk);
		});
	});
}

void AVoxelWorldActor::ProcessPendingMeshQueue()
{
	int32 i = 0;
	while (i < PendingMeshQueue.Num() && ActiveMeshTasks.Load() < MaxConcurrentMeshTasks)
	{
		UVoxelChunkComponent* Chunk = PendingMeshQueue[i];
		if (Chunk && Chunk->NeedsMesh())
		{
			PendingMeshQueue.RemoveAtSwap(i);
			MeshTaskAsync(Chunk);
		}
		else
		{
			i++;
		}
	}
}

void AVoxelWorldActor::ProcessUploadQueue()
{
	TArray<UVoxelChunkComponent*> ToUpload;
	{
		FScopeLock Lock(&UploadQueueMutex);
		const int32 Count = FMath::Min(MaxUploadsPerTick, UploadQueue.Num());
		if (Count > 0)
		{
			ToUpload.Append(UploadQueue.GetData(), Count);
			UploadQueue.RemoveAt(0, Count, EAllowShrinking::No);
		}
	}
	for (UVoxelChunkComponent* Chunk : ToUpload)
	{
		if (Chunk)
		{
			Chunk->UploadMesh(TerrainMaterial);
		}
	}
}

TSet<FChunkCoord> AVoxelWorldActor::ApplyDigSphere(FVector WorldCenter, float Radius, float Strength)
{
	const float DensityScale = Strength * (Radius / VOXEL_SIZE);
	return ApplySphereOp(WorldCenter, Radius,
		[DensityScale](float Current, float Falloff)
		{
			return Current + Falloff * DensityScale;
		}
	);
}

TSet<FChunkCoord> AVoxelWorldActor::ApplyAddSphere(FVector WorldCenter, float Radius, float Strength)
{
	const float DensityScale = Strength * (Radius / VOXEL_SIZE);
	return ApplySphereOp(WorldCenter, Radius,
		[DensityScale](float Current, float Falloff)
		{
			return Current - Falloff * DensityScale;
		}
	);
}

TSet<FChunkCoord> AVoxelWorldActor::ApplySphereOp(FVector WorldCenter, float Radius, TFunctionRef<float(float CurrentDensity, float Falloff)> DensityOp)
{
	//const FVector LocalCenter = WorldCenter - GetActorLocation();
	TSet<FChunkCoord> DirtyChunks;
	const int32 ChunkRadius = FMath::CeilToInt(Radius / CHUNK_WORLD_SIZE) + 1;
	const FChunkCoord CenterChunk = FChunkCoord::FromWorldPosition(WorldCenter);

	for (int32 DZ = -ChunkRadius; DZ <= ChunkRadius; DZ++)
	{
		for (int32 DY = -ChunkRadius; DY <= ChunkRadius; DY++)
		{
			for (int32 DX = -ChunkRadius; DX <= ChunkRadius; DX++)
			{
				const FChunkCoord CC(CenterChunk.X + DX, CenterChunk.Y + DY, CenterChunk.Z + DZ);
				UVoxelChunkComponent** ChunkPtr = Chunks.Find(CC);
				if (!ChunkPtr || !*ChunkPtr) continue;
				UVoxelChunkComponent& Chunk = **ChunkPtr;
				bool bModified = false;
				for (int32 LZ = 0; LZ < CHUNK_SAMPLE_SIZE; LZ++)
				{
					for (int32 LY = 0; LY < CHUNK_SAMPLE_SIZE; LY++)
					{
						for (int32 LX = 0; LX < CHUNK_SAMPLE_SIZE; LX++)
						{
							const FVector SampleWorld = Chunk.GetChunkWorldOrigin() +
								FVector(LX * VOXEL_SIZE, LY * VOXEL_SIZE, LZ * VOXEL_SIZE);
							const float Dist = FVector::Dist(SampleWorld, WorldCenter);
							if (Dist >= Radius) continue;
							const float Falloff = 1.0f - (Dist / Radius);
							const float OldDensity = Chunk.GetDensity(LX, LY, LZ);
							const float NewDensity = DensityOp(OldDensity, Falloff);

							Chunk.SetDensity(FVoxelCoord(LX, LY, LZ), NewDensity);

							bModified = true;
						}
					}
				}
				if (bModified) DirtyChunks.Add(CC);
			}
		}
	}
	return DirtyChunks;
}

void AVoxelWorldActor::RemeshDirtyChunks(const TSet<FChunkCoord>& DirtyChunks)
{
	for (const FChunkCoord& CC : DirtyChunks)
	{
		UVoxelChunkComponent** ChunkPtr = Chunks.Find(CC);
		if (!ChunkPtr || !*ChunkPtr) continue;
		UVoxelChunkComponent* Chunk = *ChunkPtr;
		Chunk->MarkDirty();
		MeshTaskAsync(Chunk);
	}
}

void AVoxelWorldActor::ReplayModificationLog()
{
	const int32 TotalEntries = ModificationLog.Num();
	LastReplayedIndex = FMath::Min(LastReplayedIndex, TotalEntries);
	if (LastReplayedIndex >= TotalEntries)
	{
		return;
	}
	TSet<FChunkCoord> AllDirtyChunks;

	for (int32 i = LastReplayedIndex; i < TotalEntries; i++)
	{
		const FVoxelModification& Mod = ModificationLog[i];
		TSet<FChunkCoord> DirtyChunks;

		switch (Mod.OpType)
		{
		case EVoxelOpType::DigSphere:
		{
			DirtyChunks = ApplyDigSphere(Mod.WorldCenter, Mod.Radius, Mod.Strength);
			break;
		}
		case EVoxelOpType::AddSphere:
		{
			DirtyChunks = ApplyAddSphere(Mod.WorldCenter, Mod.Radius, Mod.Strength);
			break;
		}
		}
		AllDirtyChunks.Append(DirtyChunks);
	}
	LastReplayedIndex = TotalEntries;
	RemeshDirtyChunks(AllDirtyChunks);
}

void AVoxelWorldActor::OnRep_NoiseSeed()
{
	NoiseSeed = ReplicatedSeed;
	RebuildTerrain();
}

void AVoxelWorldActor::OnRep_ModificationLog()
{
	if (Chunks.Num() == 0)
	{
		return;
	}
	ReplayModificationLog();
}
