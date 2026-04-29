

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelChunkComponent.h"
#include "VoxelWorldActor.generated.h"

class UHeightmapProcessor;

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
	int32 ViewDistance = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World")
	int32 SurfaceChunkZ = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World")
	UMaterialInterface* TerrainMaterial = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World", meta = (ClampMin = "1", ClampMax = "16"))
	int32 MaxUploadsPerTick = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World", meta = (ClampMin = "1", ClampMax = "8"))
	int32 MaxConcurrentMeshTasks = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Mesh")
	EVoxelTerrainSource TerrainSource = EVoxelTerrainSource::ProceduralNoise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
	float NoiseFrequency = 0.002f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "1.0", ClampMax = "64.0"))
	float NoiseAmplitude = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise")
	float SurfaceLevel = 16.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise")
	int32 NoiseSeed = 42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "0.0001", ClampMax = "0.1"))
	float CaveFrequency = 0.008f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Noise", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float CaveAmplitude = 3.0f;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel World|Heightmap", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float HeightmapCaveAmplitude = 3.f;

	AVoxelWorldActor();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Voxel World")
	void RebuildTerrain();

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

	virtual bool ShouldTickIfViewportsOnly() const override { return true; }
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
	UHeightmapProcessor* HeightmapProcessor = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_NoiseSeed)
	int32 ReplicatedSeed = 42;
	UPROPERTY(ReplicatedUsing = OnRep_ModificationLog)
	TArray<FVoxelModification> ModificationLog;

	int32 LastReplayedIndex = 0;
	bool bTerrainBuilt = false;
	void CreateChunkGrid();
	void DestroyChunkGrid();
	UVoxelChunkComponent* CreateChunk(const FChunkCoord& Coord);

	void DensityTaskAsync(UVoxelChunkComponent* Chunk);
	void MeshTaskAsync(UVoxelChunkComponent* Chunk);

	void ProcessPendingMeshQueue();
	void ProcessUploadQueue();

	TSet<FChunkCoord> ApplyDigSphere(FVector WorldCenter, float Radius, float Strength);
	TSet<FChunkCoord> ApplyAddSphere(FVector WorldCenter, float Radius, float Strength);
	TSet<FChunkCoord> ApplySphereOp(FVector WorldCenter, float Radius,
		TFunctionRef<float(float CurrentDensity, float Falloff)> DensityOp);
	void RemeshDirtyChunks(const TSet<FChunkCoord>& DirtyChunks);
	void ReplayModificationLog();

	UFUNCTION()
	void OnRep_NoiseSeed();
	UFUNCTION()
	void OnRep_ModificationLog();
};
