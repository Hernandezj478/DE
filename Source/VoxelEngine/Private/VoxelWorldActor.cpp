#include "VoxelWorldActor.h"
#include "MarchingCubeMesher.h"
#include "HeightmapProcessor.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Texture2D.h"
#include "Async/Async.h"
#include "Logger.h"

struct FCapturedTypeRule
{
	uint8 TypeIndex;
	int32 MinDepth;
	int32 MaxDepth;
	float NoiseFrequency;
	float RarityThreshold;
};

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
	if(HasAuthority())
	{
		if (!bHasGeneratedSeed)
		{
			GenerateNewSeed();
		}
		ReplicatedSeed = NoiseSeed;
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
	if (!bHasGeneratedSeed)
	{
		GenerateNewSeed();
		#if WITH_EDITOR
			MarkPackageDirty();
		#endif
	}
	if (!bConstructed)
	{
		RebuildTerrain();
		bConstructed = true;
	}
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

void AVoxelWorldActor::RegenerateSeed()
{
	GenerateNewSeed();
	RebuildTerrain();
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
	for (int32 CZ = -ViewDistanceZ; CZ <= ViewDistanceZ; CZ++)
	{
		for (int32 CY = -ViewDistanceXY; CY <= ViewDistanceXY; CY++)
		{
			for (int32 CX = -ViewDistanceXY; CX <= ViewDistanceXY; CX++)
			{
				const FChunkCoord Coord(
					ActorChunkCoord.X + CX, 
					ActorChunkCoord.Y + CY, 
					ActorChunkCoord.Z + CZ + SurfaceChunkZ);
				UVoxelChunkComponent* Chunk = CreateChunk(Coord);
				const int32 ChebDist = FMath::Max3(FMath::Abs(CX), FMath::Abs(CY), FMath::Abs(CZ));
				const int32 LODLevel = (ChebDist <= 1) ? 0 : (ChebDist <= 3) ? 1 : 2;
				DensityTaskAsync(Chunk, LODLevel);
			}
		}
	}
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

void AVoxelWorldActor::DensityTaskAsync(UVoxelChunkComponent* Chunk, int32 LODLevel)
{
	Chunk->SetState(EChunkState::GeneratingDensity);

	TArray<FCapturedTypeRule> CapturedRules;
	int32 OreMinDepthGlobal = INT_MAX;
	int32 OreMaxDepthGlobal = 0;

	if (VoxelTypeDataTable)
	{
		const UEnum* VoxelTypeEnum = StaticEnum<EVoxelType>();
		for (const FName& RowName : VoxelTypeDataTable->GetRowNames())
		{
			const FVoxelTypeData* Row = VoxelTypeDataTable->FindRow<FVoxelTypeData>(
				RowName, TEXT("DensityTaskAsync"), false);
			if (!Row || Row->MaxGenerationDepth <= 0)
			{
				continue;
			}
			const int64 EnumValue = VoxelTypeEnum ? VoxelTypeEnum->GetValueByNameString(RowName.ToString()) : INDEX_NONE;
			if (EnumValue == INDEX_NONE)
			{
				continue;
			}
			FCapturedTypeRule Rule;
			Rule.TypeIndex = static_cast<uint8>(EnumValue);
			Rule.MinDepth = Row->MinGenerationDepth;
			Rule.MaxDepth = Row->MaxGenerationDepth;
			Rule.RarityThreshold = Row->OreRarityThreshold;
			Rule.NoiseFrequency = Row->OreNoiseFrequency;
			CapturedRules.Add(Rule);

			if (Rule.RarityThreshold > 0.f)
			{
				OreMinDepthGlobal = FMath::Min(OreMinDepthGlobal, Rule.MinDepth);
				OreMaxDepthGlobal = FMath::Max(OreMaxDepthGlobal, Rule.MaxDepth);
			}
		}
		CapturedRules.Sort([](const FCapturedTypeRule& A, const FCapturedTypeRule& B)
			{
				return A.MinDepth > B.MinDepth;
			});
	}

	if (OreMaxDepthGlobal == 0)
	{
		OreMinDepthGlobal = INT_MAX;
		OreMaxDepthGlobal = -1;
	}

	TArray<float> CapturedHeights;
	int32 CapturedHMWidth = 0;
	int32 CapturedHMHeight = 0;
	float CapturedHMMin = 0.f;
	float CapturedHMMax = 32.f;
	bool bUseHeightmap = false;

	if (TerrainSource == EVoxelTerrainSource::Heightmap && HeightmapProcessor && HeightmapProcessor->IsLoaded())
	{
		CapturedHMWidth = HeightmapProcessor->GetImageWidth();
		CapturedHMHeight = HeightmapProcessor->GetImageHeight();
		CapturedHMMin = HeightmapProcessor->MinHeightVoxels;
		CapturedHMMax = HeightmapProcessor->MaxHeightVoxels;
		bUseHeightmap = true;

		CapturedHeights.SetNumUninitialized(CapturedHMWidth * CapturedHMHeight);
		for (int32 PY = 0; PY < CapturedHMHeight; PY++)
		{
			for (int32 PX = 0; PX < CapturedHMWidth; PX++)
			{
				CapturedHeights[PY * CapturedHMWidth + PX] = HeightmapProcessor->GetNormalizedValue(PX, PY);
			}
		}
	}

	const EVoxelTerrainSource CapturedSource = TerrainSource;
	const FVector ChunkOrigin = Chunk->GetChunkWorldOrigin();
	const FVector CapturedActorLocation = GetActorLocation();
	const float CapturedFreq = NoiseFrequency;
	const float CapturedAmp = NoiseAmplitude;
	const float CapturedSurface = SurfaceLevel;
	const int32 CapturedSeed = NoiseSeed;
	const float CapturedCaveFreq = CaveFrequency;
	const float CapturedCaveThresh = CaveCarveThreshold;
	const float CapturedCaveDepth = static_cast<float>(CaveDepthThreshold);

	const bool CapturedHMCaves = bHeightmapAddCaves;
	const float CapturedHMCaveFreq = HeightmapCaveFrequency;
	const float CapturedHMCaveThresh = HeightmapCaveCarveThreshold;

	const int32 SurfaceOctaves = (LODLevel == 0) ? 4 : (LODLevel == 1) ? 3 : 2;
	const int32 CaveOctaves = (LODLevel == 0) ? 3 : (LODLevel == 1) ? 2 : 1;

	TWeakObjectPtr<UVoxelChunkComponent> WeakChunk(Chunk);
	TWeakObjectPtr<AVoxelWorldActor> WeakSelf(this);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
		[=, Rules = MoveTemp(CapturedRules), Heights = MoveTemp(CapturedHeights),
		OreMinD = OreMinDepthGlobal, OreMaxD = OreMaxDepthGlobal]() mutable
		{
			TArray<float> LocalDensities;
			TArray<uint8> LocalTypes;
			LocalDensities.SetNumUninitialized(CHUNK_SAMPLE_COUNT);
			LocalTypes.SetNumUninitialized(CHUNK_SAMPLE_COUNT);

			TArray<float> SurfaceHeightCache;
			SurfaceHeightCache.SetNumUninitialized(CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE);

			auto HMSampleClamped = [&Heights, CapturedHMWidth, CapturedHMHeight](int32 PX, int32 PY) -> float
				{
					PX = FMath::Clamp(PX, 0, CapturedHMWidth - 1);
					PY = FMath::Clamp(PY, 0, CapturedHMHeight - 1);
					return Heights[PY * CapturedHMWidth + PX];
				};

			auto HMSampleBilinear = [&HMSampleClamped](float FX, float FY) -> float
				{
					const int32 X0 = static_cast<int32>(FX);
					const int32 Y0 = static_cast<int32>(FY);
					const float TX = FX - X0;
					const float TY = FY - Y0;
					return FMath::Lerp(
						FMath::Lerp(HMSampleClamped(X0, Y0), HMSampleClamped(X0 + 1, Y0), TX),
						FMath::Lerp(HMSampleClamped(X0, Y0 + 1), HMSampleClamped(X0 + 1, Y0 + 1), TX),
						TY);
				};
			auto HMGetHeightAtWorld = [&](float WX, float WY, float OriginX, float OriginY) -> float
				{
					const float FX = (WX - OriginX) / VOXEL_SIZE;
					const float FY = (WY - OriginY) / VOXEL_SIZE;
					const float T = HMSampleBilinear(FX, FY);
					return FMath::Lerp(CapturedHMMin, CapturedHMMax, T);
				};
		auto ApplyCaves = [](float SurfDensity, float CaveNoise3D, float CarveThreshold, float DepthThreshold) -> float
			{
				if (SurfDensity >= -DepthThreshold) 
				{
					return SurfDensity;
				}
				const float Normalized = CaveNoise3D * 0.5f + 0.5f;
				if (Normalized <= CarveThreshold) 
				{
					return SurfDensity;
				}
				return 1.0f;
			};

		if (bUseHeightmap && Heights.Num() > 0)
		{
			
			const float HalfW = CapturedHMWidth * VOXEL_SIZE * 0.5f;
			const float HalfH = CapturedHMHeight * VOXEL_SIZE * 0.5f;
			const float OriginX = -HalfW;
			const float OriginY = -HalfH;
			for (int32 LY = 0; LY < CHUNK_SAMPLE_SIZE; LY++)
			{
				for (int32 LX = 0; LX < CHUNK_SAMPLE_SIZE; LX++)
				{
					const float WX = ChunkOrigin.X + LX * VOXEL_SIZE;
					const float WY = ChunkOrigin.Y + LY * VOXEL_SIZE;
					const float TerrainH = HMGetHeightAtWorld(WX, WY, OriginX, OriginY);
					SurfaceHeightCache[LY * CHUNK_SAMPLE_SIZE + LX] = TerrainH;

					for (int32 LZ = 0; LZ < CHUNK_SAMPLE_SIZE; LZ++)
					{
						const float WZ = ChunkOrigin.Z + LZ * VOXEL_SIZE;
						const float RelativeZ = WZ - CapturedActorLocation.Z;
						const float SurfDensity = (RelativeZ / VOXEL_SIZE) - TerrainH;
						float FinalDensity = SurfDensity;
						if (CapturedHMCaves && SurfDensity < -CapturedCaveDepth)
						{
							const float CN = FBM(
								WX * CapturedHMCaveFreq,
								WY * CapturedHMCaveFreq,
								WZ * CapturedHMCaveFreq,
								CapturedSeed + 999, 3, 1.f, 1.f, 2.f, 0.6f);
							FinalDensity = ApplyCaves(SurfDensity, CN, CapturedHMCaveThresh, CapturedCaveDepth);
						}
						LocalDensities[FVoxelCoord(LX, LY, LZ).ToIndex()] = FinalDensity;
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
					SurfaceHeightCache[LY * CHUNK_SAMPLE_SIZE + LX] = SurfaceH;
					for (int32 LZ = 0; LZ < CHUNK_SAMPLE_SIZE; LZ++)
					{
						const float WZ = ChunkOrigin.Z + LZ * VOXEL_SIZE;
						const float RelativeZ = WZ - CapturedActorLocation.Z;
						const float SurfDensity = (RelativeZ / VOXEL_SIZE) - SurfaceH;
						float FinalDensity = SurfDensity;
						if (SurfDensity < -CapturedCaveDepth)
						{
							const float CN = FBM(
								WX * CapturedCaveFreq,
								WY * CapturedCaveFreq,
								WZ * CapturedCaveFreq,
								CapturedSeed + 999, 3, 1.f, 1.f, 2.f, 0.6f);
							FinalDensity = ApplyCaves(SurfDensity, CN, CapturedCaveThresh, CapturedCaveDepth);
						}
						LocalDensities[FVoxelCoord(LX, LY, LZ).ToIndex()] = FinalDensity;
					}
				}
			}
		}

		for (int32 LY = 0; LY < CHUNK_SAMPLE_SIZE; LY++)
		{
			for (int32 LX = 0; LX < CHUNK_SAMPLE_SIZE; LX++)
			{
				const float SurfaceH = SurfaceHeightCache[LY * CHUNK_SAMPLE_SIZE + LX];
				const float WX = ChunkOrigin.X + LX * VOXEL_SIZE;
				const float WY = ChunkOrigin.Y + LY * VOXEL_SIZE;

				for (int32 LZ = 0; LZ < CHUNK_SAMPLE_SIZE; LZ++)
				{
					const int32 Idx = FVoxelCoord(LX, LY, LZ).ToIndex();
					if (LocalDensities[Idx] >= ISO_LEVEL)
					{
						LocalTypes[Idx] = static_cast<uint8>(EVoxelType::Air);
						continue;
					}
					const float LocalZ = ChunkOrigin.Z - CapturedActorLocation.Z;
					const float VoxelZ = (LocalZ / VOXEL_SIZE) + static_cast<float>(LZ);
					const float Depth = SurfaceH - VoxelZ;
					const float WZ = ChunkOrigin.Z + LZ * VOXEL_SIZE;
					const int32 DepthVoxels = FMath::FloorToInt(Depth);

					EVoxelType FinalType = EVoxelType::Rock;

					for (const FCapturedTypeRule& Rule : Rules)
					{
						if (Depth < static_cast<float>(Rule.MinDepth) 
							|| Depth >= static_cast<float>(Rule.MaxDepth))
						{
							continue;
						}
						if (Rule.RarityThreshold <= 0.f)
						{
							// Solid layer - assign as base, keep checking for ores
							FinalType = static_cast<EVoxelType>(Rule.TypeIndex);
						}
						else
						{
							// Ore candidate - noise check
							const int32 OreSeed = CapturedSeed + 1000 + static_cast<int32>(Rule.TypeIndex) * 37;
							if (DepthVoxels < OreMinD || DepthVoxels > OreMaxD)
							{
								continue;
							}
							const float OreSample = FBM(
								WX * Rule.NoiseFrequency,
								WY * Rule.NoiseFrequency,
								WZ * Rule.NoiseFrequency,
								OreSeed, 2, 1.f, 1.f, 2.f, 0.5f);
							const float Normalized = OreSample * 0.5f + 0.5f;
							if (Normalized > Rule.RarityThreshold)
							{
								FinalType = static_cast<EVoxelType>(Rule.TypeIndex);
								break;// ore found
							}
						}
					}
					LocalTypes[Idx] = static_cast<uint8>(FinalType);
				}
			}
		}


		AsyncTask(ENamedThreads::GameThread, [WeakChunk, WeakSelf,
		Densities = MoveTemp(LocalDensities),
		Types = MoveTemp(LocalTypes)]() mutable
		{
			UVoxelChunkComponent* Chunk = WeakChunk.Get();
			AVoxelWorldActor* Self = WeakSelf.Get();
			if (!Chunk || !Self)
			{
				return;
			}
			Chunk->ApplyGeneratedData(MoveTemp(Densities), MoveTemp(Types));
			Chunk->SetState(EChunkState::PendingMesh);
			Self->PendingMeshQueue.Add(Chunk);

			static const FChunkCoord FaceOffsets[6] = 
			{
				FChunkCoord( 1,  0,  0),
				FChunkCoord(-1,  0,  0),
				FChunkCoord( 0,  1,  0),
				FChunkCoord( 0, -1,  0),
				FChunkCoord( 0,  0,  1),
				FChunkCoord( 0,  0, -1)
			};
			const FChunkCoord CC = Chunk->GetChunkCoord();
			for (const FChunkCoord& Offset : FaceOffsets)
			{
				const FChunkCoord NC(CC.X + Offset.X, CC.Y + Offset.Y, CC.Z + Offset.Z);
				UVoxelChunkComponent* const* NeighborPtr = Self->Chunks.Find(NC);
				if (!NeighborPtr || !*NeighborPtr)
				{
					continue;
				}
				UVoxelChunkComponent* Neighbor = *NeighborPtr;
				const EChunkState NeighborState = Neighbor->GetState();
				if (NeighborState == EChunkState::Ready || NeighborState == EChunkState::PendingUpload)
				{
					Neighbor->SetState(EChunkState::PendingMesh);
					Self->PendingMeshQueue.Add(Neighbor);
				}
			}
		});
	});
}

void AVoxelWorldActor::MeshTaskAsync(UVoxelChunkComponent* Chunk)
{
	Chunk->SetState(EChunkState::GeneratingMesh);
	++ActiveMeshTasks;

	TArray<float> Densities(Chunk->GetDensityData(), CHUNK_SAMPLE_COUNT);
	TArray<uint8> LocalTypes(Chunk->GetVoxelTypeData(), CHUNK_SAMPLE_COUNT);
	TWeakObjectPtr<UVoxelChunkComponent> WeakChunk(Chunk);
	TWeakObjectPtr<AVoxelWorldActor> WeakSelf(this);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakChunk, WeakSelf,
	Densities = MoveTemp(Densities), Types = MoveTemp(LocalTypes)]()mutable
	{
		FChunkMeshData NewMeshData;
		FMarchingCubeMesher::MeshChunk(Densities, Types, NewMeshData);
		AsyncTask(ENamedThreads::GameThread,
			[WeakChunk, WeakSelf, MeshData = MoveTemp(NewMeshData)]() mutable
			{
				UVoxelChunkComponent* Chunk = WeakChunk.Get();
				AVoxelWorldActor* Self = WeakSelf.Get();
				if (!Chunk || !Self)
				{
					--Self->ActiveMeshTasks;
					return;
				}
				{
					FScopeLock Lock(&Chunk->MeshDataMutex);
					Chunk->PendingMeshData = MoveTemp(MeshData);
				}
				Chunk->SetState(EChunkState::PendingUpload);
				--Self->ActiveMeshTasks;

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

void AVoxelWorldActor::GenerateNewSeed()
{
	FRandomStream Stream;
	Stream.GenerateNewSeed();
	NoiseSeed = Stream.GetCurrentSeed();
	bHasGeneratedSeed = true;
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
