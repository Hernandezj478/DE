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
	ProcessPendingChunkCreations();
	UWorld* World = GetWorld();
	if (!World) return;
	FVector ObserverPos = GetActorLocation();
	if (World->IsGameWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = PC->GetPawnOrSpectator())
			{
				ObserverPos = Pawn->GetActorLocation();
			}
		}
	}
	UpdateStreaming(ObserverPos);
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

void AVoxelWorldActor::UpdateStreaming(FVector ObserverPosition)
{
	const float ThresholdSq = FMath::Square(CHUNK_WORLD_SIZE * StreamingUpdateThreshold);
	if (FVector::DistSquared(ObserverPosition, LastStreamPosition) < ThresholdSq)
	{
		return;
	}
	
	const FChunkCoord ObsCC = GetObserverChunkCoord(ObserverPosition);
	if (ObsCC == LastObserverChunkCoord)
	{
		LastStreamPosition = ObserverPosition;
		return;
	}

	LastStreamPosition = ObserverPosition;
	LastObserverChunkCoord = ObsCC;

	const int32 OuterXY = InnerRadiusExtent + OuterRadiusExtent;
	const int32 OuterZ = ViewDistanceZ + OuterRadiusExtent;

	TSet<FChunkCoord> DesiredChunks;
	DesiredChunks.Reserve((OuterXY * 2 + 1) * (OuterXY * 2 + 1) * (OuterXY * 2 + 1));

	for (int32 DZ = -OuterZ; DZ <= OuterZ; DZ++) 
	{
		for (int32 DY = -OuterXY; DY <= OuterXY; DY++)
		{
			for (int32 DX = -OuterXY; DX <= OuterXY; DX++)
			{
				DesiredChunks.Add(FChunkCoord(ObsCC.X + DX, ObsCC.Y + DY, ObsCC.Z + DZ));
			}
		}
	}
	TArray<FChunkCoord> ToDestroy;
	for (const TPair<FChunkCoord, UVoxelChunkComponent*>& Pair : Chunks)
	{
		if (!DesiredChunks.Contains(Pair.Key))
		{
			ToDestroy.Add(Pair.Key);
		}
	}
	for (const FChunkCoord& CC : ToDestroy)
	{
		DestroyChunk(CC);
	}
	PendingChunkCreations.RemoveAll([&](const TPair<float, FChunkCoord>& Entry)
		{
			return !DesiredChunks.Contains(Entry.Value);
		});

	for (const FChunkCoord& CC : DesiredChunks)
	{
		const int32 ChebXY = FMath::Max(FMath::Abs(CC.X - ObsCC.X), FMath::Abs(CC.Y - ObsCC.Y));
		const int32 ChebZ = FMath::Abs(CC.Z - ObsCC.Z);
		const bool bInner = (ChebXY <= InnerRadiusExtent && ChebZ <= ViewDistanceZ);

		UVoxelChunkComponent** ExistingPtr = Chunks.Find(CC);
		if (!ExistingPtr || !*ExistingPtr)
		{
			const bool bAlreadyQueued = PendingChunkCreations.ContainsByPredicate([&CC](const TPair<float, FChunkCoord>& E)
			{
				return E.Value == CC;
			});
			if (!bAlreadyQueued)
			{
				const float DistSq = static_cast<float>(
					(CC.X - ObsCC.X) * (CC.X - ObsCC.X) +
					(CC.Y - ObsCC.Y) * (CC.Y - ObsCC.Y) +
					(CC.Z - ObsCC.Z) * (CC.Z - ObsCC.Z));
				PendingChunkCreations.Add({ DistSq, CC });
			}
		}
		else
		{
			UVoxelChunkComponent* Chunk = *ExistingPtr;
			const EChunkState S = Chunk->GetState();
			if (bInner && S == EChunkState::DensityReady)
			{
				Chunk->SetState(EChunkState::PendingMesh);
				PendingMeshQueue.Add(Chunk);
			}
			else if(!bInner && (S == EChunkState::Ready || S == EChunkState::Dirty))
			{
				Chunk->ClearMesh();
			}
		}
	}
	PendingChunkCreations.Sort([](const TPair<float, FChunkCoord>& A, const TPair<float, FChunkCoord>& B)
	{
		return A.Key < B.Key;
	});
}

void AVoxelWorldActor::BuildFarTerrain()
{
	if (bFarTerrainBuilding.Load())
	{
		return;
	}
	bFarTerrainBuilding = true;
	if (!FarTerrainMesh)
	{
		FarTerrainMesh = NewObject<UProceduralMeshComponent>(this, TEXT("FarTerrainMesh"));
		FarTerrainMesh->SetupAttachment(GetRootComponent());
		FarTerrainMesh->RegisterComponent();
		FarTerrainMesh->SetCanEverAffectNavigation(false);
		FarTerrainMesh->bUseAsyncCooking = false;
		FarTerrainMesh->bUseComplexAsSimpleCollision = false;
	}
	const int32 Res = FMath::Max(4, FarTerrainResolution);
	const float Extent = FarTerrainRadius;
	const float Step = (Extent * 2.f) / static_cast<float>(Res);
	const FVector ActorLoc = GetActorLocation();
	const float InnerExclusionRadius = (InnerRadiusExtent /*+ OuterRadiusExtent*/) * CHUNK_WORLD_SIZE;
	const EVoxelTerrainSource Source = TerrainSource;
	const float CapturedFreq = NoiseFrequency;
	const float CapturedAmp = NoiseAmplitude;
	const float CapturedSurface = SurfaceLevel;
	const int32 CapturedSeed = NoiseSeed;	
	UMaterialInterface* Material = FarTerrainMaterial;

	TArray<float> CapturedHeights;
	int32 HMWidth = 0;
	int32 HMHeight = 0;
	float HMMin = 0.f;
	float HMMax = 32.f;
	bool bUseHM = false;

	if (Source == EVoxelTerrainSource::Heightmap && HeightmapProcessor && HeightmapProcessor->IsLoaded())
	{
		HMWidth = HeightmapProcessor->GetImageWidth();
		HMHeight = HeightmapProcessor->GetImageHeight();
		HMMin = HeightmapProcessor->MinHeightVoxels;
		HMMax = HeightmapProcessor->MaxHeightVoxels;
		bUseHM = true;

		CapturedHeights.SetNumUninitialized(HMWidth * HMHeight);
		for (int32 PY = 0; PY < HMHeight; PY++)
		{
			for (int32 PX = 0; PX < HMWidth; PX++)
			{
				CapturedHeights[PY * HMWidth + PX] = HeightmapProcessor->GetNormalizedValue(PX, PY);
			}
		}
	}
	TWeakObjectPtr<AVoxelWorldActor> WeakSelf(this);
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
	[WeakSelf, Res, Extent, Step, ActorLoc, InnerExclusionRadius,
	Source, CapturedFreq, CapturedAmp, CapturedSurface, CapturedSeed,
	Material, bUseHM, HMWidth, HMHeight, HMMin, HMMax,
	Heights = MoveTemp(CapturedHeights)]() mutable
	{
		const int32 Stride = Res + 1;
		auto HMSampleClamped = [&Heights, HMWidth, HMHeight](int32 PX, int32 PY) -> float
		{
			PX = FMath::Clamp(PX, 0, HMWidth - 1);
			PY = FMath::Clamp(PY, 0, HMHeight - 1);
			return Heights[PY * HMWidth + PX];
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
		auto SampleHeight = [&](float WX, float WY) -> float
		{
			if (bUseHM && Heights.Num() > 0)
			{
				const float HalfW = HMWidth * VOXEL_SIZE * 0.5f;
				const float HalfH = HMHeight * VOXEL_SIZE * 0.5f;
				const float FX = (WX + HalfW) / VOXEL_SIZE;
				const float FY = (WY + HalfW) / VOXEL_SIZE;
				const float T = HMSampleBilinear(FX, FY);
				return FMath::Lerp(HMMin, HMMax, T);
			}
			const float N = FBM(WX * CapturedFreq, WY * CapturedFreq, 0.f, CapturedSeed, 4, 1.f, 1.f, 2.f, 0.5f);
			return CapturedSurface + N * CapturedAmp;
		};
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs;
		TArray<FColor> Colors;

		Vertices.Reserve(Stride * Stride);
		Triangles.Reserve(Stride * Stride * 6);
		Normals.Reserve(Stride * Stride);
		UVs.Reserve(Stride * Stride);

		for (int32 Row = 0; Row <= Res; Row++)
		{
			for (int32 Col = 0; Col <= Res; Col++)
			{
				const float LocalX = -Extent + Col * Step;
				const float LocalY = -Extent + Row * Step;
				const float WorldX = ActorLoc.X + LocalX;
				const float WorldY = ActorLoc.Y + LocalY;

				const float SurfaceH = SampleHeight(WorldX, WorldY);
				const float WorldZ = SurfaceH * VOXEL_SIZE;
				const float LocalZ = WorldZ - ActorLoc.Z;

				Vertices.Add(FVector(LocalX, LocalY, LocalZ));
				UVs.Add(FVector2D(static_cast<float>(Col) / Res, static_cast<float>(Row) / Res));
				Normals.Add(FVector(0.f, 0.f, 1.f));
				Colors.Add(FColor::White);
			}
		}
		for (int32 Row = 0; Row < Res; Row++)
		{
			for (int32 Col = 0; Col < Res; Col++)
			{
				const float CX = -Extent + (Col + 0.5f) * Step;
				const float CY = -Extent + (Row + 0.5f) * Step;

				if (FMath::Max(FMath::Abs(CX), FMath::Abs(CY)) < InnerExclusionRadius)
				{
					continue;
				}
				const int32 I00 = Row * Stride + Col;
				const int32 I10 = Row * Stride + Col + 1;
				const int32 I01 = (Row + 1) * Stride + Col;
				const int32 I11 = (Row + 1) * Stride + Col + 1;

				Triangles.Add(I00);
				Triangles.Add(I01);
				Triangles.Add(I10);
				Triangles.Add(I10);
				Triangles.Add(I01);
				Triangles.Add(I11);
			}
		}
		for (int32 Row = 1; Row < Res; Row++)
		{
			for (int32 Col = 1; Col < Res; Col++)
			{
				const int32 I = Row * Stride + Col;
				const FVector& V = Vertices[I];
				const FVector& VX = Vertices[Row * Stride + Col + 1];
				const FVector& VY = Vertices[(Row + 1) * Stride + Col];
				const FVector N = FVector::CrossProduct(VX - V, VY - V).GetSafeNormal();
				if (!N.IsNearlyZero())
				{
					Normals[I] = N;
				}
			}
		}
		AsyncTask(ENamedThreads::GameThread,
		[WeakSelf, Material,
		Verts = MoveTemp(Vertices),
		Tris = MoveTemp(Triangles),
		Norms = MoveTemp(Normals),
		UVData = MoveTemp(UVs),
		ColData = MoveTemp(Colors)]() mutable
		{
			AVoxelWorldActor* Self = WeakSelf.Get();
			if (!Self || !Self->FarTerrainMesh)
			{
				return;
			}
			Self->FarTerrainMesh->CreateMeshSection(
			0, Verts, Tris, Norms, UVData, ColData, TArray<FProcMeshTangent>(), false);
			if (Material)
			{
				Self->FarTerrainMesh->SetMaterial(0, Material);
			}
			Self->bFarTerrainBuilding = false;
		});
	});
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

FChunkCoord AVoxelWorldActor::GetObserverChunkCoord(const FVector& ObserverPosition) const
{
	return FChunkCoord::FromWorldPosition(ObserverPosition);
}

void AVoxelWorldActor::ProcessPendingChunkCreations()
{
	if (PendingChunkCreations.IsEmpty())
	{
		return;
	}
	const FChunkCoord ObsCC = LastObserverChunkCoord;
	int32 Created = 0;
	while (!PendingChunkCreations.IsEmpty() && Created < MaxChunkCreatesPerTick)
	{
		const TPair<float, FChunkCoord> Entry = PendingChunkCreations[0];
		PendingChunkCreations.RemoveAt(0, 1, EAllowShrinking::No);
		const FChunkCoord& CC = Entry.Value;
		if (Chunks.Contains(CC))
		{
			continue;
		}
		const int32 ChebXY = FMath::Max(FMath::Abs(CC.X - ObsCC.X), FMath::Abs(CC.Y - ObsCC.Y));
		const int32 ChebZ = FMath::Abs(CC.Z - ObsCC.Z);
		const bool bInner = (ChebXY <= InnerRadiusExtent && ChebZ <= ViewDistanceZ);
		UpdateChunk(CC, bInner);
		Created++;
	}
}

UVoxelChunkComponent* AVoxelWorldActor::UpdateChunk(const FChunkCoord& Coord, bool bInnerZone)
{
	if (UVoxelChunkComponent** Found = Chunks.Find(Coord))
	{
		return *Found;
	}
	UVoxelChunkComponent* Chunk = CreateChunk(Coord);
	const FChunkCoord ObsCC = GetObserverChunkCoord(LastStreamPosition);
	const int32 ChebXY = FMath::Max(FMath::Abs(Coord.X - ObsCC.X), FMath::Abs(Coord.Y - ObsCC.Y));
	const int32 LODLevel = (ChebXY <= 1) ? 0 : (ChebXY <= 3) ? 1 : 2;
	DensityTaskAsync(Chunk, LODLevel, bInnerZone);
	return Chunk;
}

void AVoxelWorldActor::DestroyChunk(const FChunkCoord& Coord)
{
	UVoxelChunkComponent** Found = Chunks.Find(Coord);
	if (!Found || !*Found)
	{
		return;
	}
	UVoxelChunkComponent* Chunk = *Found;
	if (UProceduralMeshComponent* Mesh = Chunk->GetMeshComponent())
	{
		Mesh->DestroyComponent();
	}
	Chunk->DestroyComponent();
	Chunks.Remove(Coord);
}

void AVoxelWorldActor::CreateChunkGrid()
{
	const FVector InitialPos = GetActorLocation();
	const FChunkCoord ObsCC = GetObserverChunkCoord(InitialPos);
	const int32 OuterXY = InnerRadiusExtent + OuterRadiusExtent;
	const int32 OuterZ = ViewDistanceZ + OuterRadiusExtent;
	PendingChunkCreations.Empty();
	PendingChunkCreations.Reserve((OuterXY * 2 + 1) * (OuterXY * 2 + 1) * (OuterZ * 2 + 1));
	//const FChunkCoord ActorChunkCoord = FChunkCoord::FromWorldPosition(GetActorLocation());
	for (int32 DZ = -OuterZ; DZ <= OuterZ; DZ++)
	{
		for (int32 DY = -OuterXY; DY <= OuterXY; DY++)
		{
			for (int32 DX = -OuterXY; DX <= OuterXY; DX++)
			{
				const FChunkCoord Coord(ObsCC.X + DX, ObsCC.Y + DY, ObsCC.Z + DZ);
				const float DistSq = static_cast<float>(DX * DX + DY * DY + DZ * DZ);
				PendingChunkCreations.Add({ DistSq, Coord });
				//UVoxelChunkComponent* Chunk = CreateChunk(Coord);
				//const int32 ChebXY = FMath::Max(FMath::Abs(CX), FMath::Abs(CY));
				//const int32 ChebZ = FMath::Abs(CZ);
				//const bool bInner = (ChebXY <= InnerRadiusExtent && ChebZ <= ViewDistanceZ);
			}
		}
	}
	PendingChunkCreations.Sort([](const TPair<float, FChunkCoord>& A, const TPair<float, FChunkCoord>& B)
	{
		return A.Key < B.Key;
	});
	LastObserverChunkCoord = ObsCC;
	LastStreamPosition = InitialPos;
	//BuildFarTerrain();
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

void AVoxelWorldActor::DensityTaskAsync(UVoxelChunkComponent* Chunk, int32 LODLevel, bool bInnerZone)
{
	LOG_MSG(DEBUG, "Calculating Density");
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
		Types = MoveTemp(LocalTypes),
		bIsInner = bInnerZone]() mutable
		{
			UVoxelChunkComponent* Chunk = WeakChunk.Get();
			AVoxelWorldActor* Self = WeakSelf.Get();
			if (!Chunk || !Self)
			{
				return;
			}
			Chunk->ApplyGeneratedData(MoveTemp(Densities), MoveTemp(Types));
			if (bIsInner)
			{
				Chunk->SetState(EChunkState::PendingMesh);
				Self->PendingMeshQueue.Add(Chunk);
			}
			else
			{
				Chunk->SetState(EChunkState::DensityReady);
			}

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

float AVoxelWorldActor::SampleSurfaceHeight(float WorldX, float WorldY) const
{
	if (TerrainSource == EVoxelTerrainSource::Heightmap && HeightmapProcessor && HeightmapProcessor->IsLoaded())
	{
		const float HalfW = HeightmapProcessor->GetImageWidth() * VOXEL_SIZE * 0.5;
		const float HalfH = HeightmapProcessor->GetImageHeight() * VOXEL_SIZE * 0.5f;
		return HeightmapProcessor->GetHeightAtWorld(WorldX, WorldY, -HalfW, -HalfH, VOXEL_SIZE);
	}
	const float SurfNoise = FBM(WorldX * NoiseFrequency, WorldY * NoiseFrequency, 0.f, NoiseSeed, 4, 1.f, 1.f, 2.f, 0.5f);
	return SurfaceLevel + SurfNoise * NoiseAmplitude;
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
