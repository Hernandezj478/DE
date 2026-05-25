

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunkComponent.h"
#include "VoxelWorldActor.generated.h"

class UHeightmapProcessor;
class UDataTable;

UENUM(BlueprintType)
enum class EVoxelTerrainSource : uint8
{
	ProceduralNoise UMETA(DisplayName = "Procedural Noise"),
	Heightmap		UMETA(DisplayName = "Heightmap Image")
};


UCLASS()
class VOXELENGINE_API AVoxelWorldActor : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World", meta = (ClampMin = "1", ClampMax = "16"))
	int32 ViewDistanceZ = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World", meta = (ClampMin = "1", ClampMax = "16"))
	int32 InnerRadiusExtent = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VoxelWorld|Streaming", meta = (ClampMin = "0", ClampMax = "8"))
	int32 OuterRadiusExtent = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Streaming", meta = (ClampMin = "1", ClampMax = "8"))
	int32 MaxChunkOpsPerTick = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Streaming", meta = (ClampMin = "1", ClampMax = "16"))
	int32 MaxChunkCreatesPerTick = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Streaming", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float StreamingUpdateThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Far Terrain", meta = (ClampMin = "1000.0"))
	float FarTerrainRadius = 50000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Far Terrain", meta = (ClampMin = "16", ClampMax = "256"))
	int32 FarTerrainResolution = 64;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Far Terrain")
	UMaterialInterface* FarTerrainMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World")
	UMaterialInterface* TerrainMaterial = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World", meta = (ClampMin = "1", ClampMax = "16"))
	int32 MaxUploadsPerTick = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World", meta = (ClampMin = "1", ClampMax = "8"))
	int32 MaxConcurrentMeshTasks = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Mesh")
	EVoxelTerrainSource TerrainSource = EVoxelTerrainSource::ProceduralNoise;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Voxel World|Types")
	UDataTable* VoxelTypeDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
	float NoiseFrequency = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "1.0", ClampMax = "64.0"))
	float NoiseAmplitude = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise")
	float SurfaceLevel = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise")
	int32 NoiseSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
	float CaveFrequency = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "1", ClampMax = "32"))
	int32 CaveDepthThreshold = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CaveCarveThreshold = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Heightmap")
	UTexture2D* HeightmapTexture = nullptr;

	// Terrain height for black pixels
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Heightmap")
	float HeightmapMinVoxels = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Heightmap")
	float HeightmapMaxVoxels = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Heightmap")
	bool bHeightmapAddCaves = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Heightmap", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
	float HeightmapCaveFrequency = 0.0008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Heightmap", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HeightmapCaveCarveThreshold = 0.72f;

	AVoxelWorldActor();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Voxel World")
	void RebuildTerrain();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Voxel World")
	void RegenerateSeed();

	UFUNCTION(BlueprintCallable, Category = "Voxel World|Streaming")
	void UpdateStreaming(FVector ObserverPosition);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Voxel World|Streaming")
	void BuildFarTerrain();

	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain|Modification")
	void DigSphere(FVector WorldCenter, float Radius, float Strength = 1.0f);
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain|Modification")
	void AddSphere(FVector WorldCenter, float Radius, float Strength = 1.0f);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyDig(FVector WorldCenter, float Radius, float Strength);
	void MulticastApplyDig_Implementation(FVector WorldCenter, float Radius, float Strength);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyAdd(FVector WorldCenter, float Radius, float Strength);
	void MulticastApplyAdd_Implementation(FVector WorldCenter, float Radius, float Strength);

	virtual bool ShouldTickIfViewportsOnly() const override 
	{ 
		return true; 
	}
protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
private:

	TMap<FChunkCoord, UVoxelChunkComponent*> Chunks;
	TArray<UVoxelChunkComponent*> PendingMeshQueue;

	TArray<UVoxelChunkComponent*> UploadQueue;
	FCriticalSection UploadQueueMutex;

	TAtomic<int32> ActiveMeshTasks{ 0 };
	static constexpr int32 MaxModificationLogSize = 10000;

	UPROPERTY()
	UProceduralMeshComponent* FarTerrainMesh = nullptr;
	TArray<TPair<float, FChunkCoord>> PendingChunkCreations;
	FChunkCoord LastObserverChunkCoord = FChunkCoord(INT_MAX, INT_MAX, INT_MAX);
	FVector LastStreamPosition = FVector(FLT_MAX);

	TAtomic<bool> bFarTerrainBuilding{ false };

	UPROPERTY()
	UHeightmapProcessor* HeightmapProcessor = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_NoiseSeed)
	int32 ReplicatedSeed = 0;
	UPROPERTY(ReplicatedUsing = OnRep_ModificationLog)
	TArray<FVoxelModification> ModificationLog;

	int32 LastReplayedIndex = 0;
	bool bTerrainBuilt = false;
	bool bHasGeneratedSeed = false;
	bool bConstructed = false;
	FChunkCoord GetObserverChunkCoord(const FVector& ObserverPosition) const;
	void ProcessPendingChunkCreations();
	UVoxelChunkComponent* UpdateChunk(const FChunkCoord& Coord, bool bInnerZone);

	void DestroyChunk(const FChunkCoord& Coord);

	void CreateChunkGrid();
	void DestroyChunkGrid();
	UVoxelChunkComponent* CreateChunk(const FChunkCoord& Coord);

	void DensityTaskAsync(UVoxelChunkComponent* Chunk, int32 LODLevel, bool bInnerZone);
	void MeshTaskAsync(UVoxelChunkComponent* Chunk);

	void ProcessPendingMeshQueue();
	void ProcessUploadQueue();

	float SampleSurfaceHeight(float WorldX, float WorldY) const;

	TSet<FChunkCoord> ApplyDigSphere(FVector WorldCenter, float Radius, float Strength);
	TSet<FChunkCoord> ApplyAddSphere(FVector WorldCenter, float Radius, float Strength);
	TSet<FChunkCoord> ApplySphereOp(FVector WorldCenter, float Radius,
		TFunctionRef<float(float CurrentDensity, float Falloff)> DensityOp);
	void RemeshDirtyChunks(const TSet<FChunkCoord>& DirtyChunks);
	void ReplayModificationLog();

	void GenerateNewSeed();

	UFUNCTION()
	void OnRep_NoiseSeed();
	UFUNCTION()
	void OnRep_ModificationLog();
};
