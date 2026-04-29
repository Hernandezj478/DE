#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoxelTypes.h"
#include "MarchingCubeMesher.h"
#include "VoxelChunkComponent.generated.h"

UENUM(BlueprintType)
enum class EChunkState : uint8
{
	Uninitialized,
	GeneratingDensity,
	PendingMesh,
	GeneratingMesh,
	PendingUpload,
	Ready,
	Dirty
};

class UProceduralMeshComponent;

UCLASS( ClassGroup="Voxel", meta = (BlueprintSpawnableComponent))
class VOXELENGINE_API UVoxelChunkComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	FChunkCoord ChunkCoord;
	FChunkMeshData PendingMeshData;
	FCriticalSection MeshDataMutex;

	UVoxelChunkComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FORCEINLINE const FChunkCoord& GetChunkCoord() const { return ChunkCoord; }
	FORCEINLINE FVector GetChunkWorldOrigin() const { return ChunkCoord.ToWorldPosition(); }

	void SetState(EChunkState NewState){ State = NewState; }
	void MarkDirty()				   { State = EChunkState::Dirty; }

	UFUNCTION(BlueprintPure, Category = "Voxel Chunk")
	EChunkState GetState() const{ return State; }
	bool IsReady() const		{ return State == EChunkState::Ready; }
	bool IsDirty() const		{ return State == EChunkState::Dirty; }
	bool IsGenerating() const	{ return State == EChunkState::GeneratingDensity 
									  || State == EChunkState::GeneratingMesh; }
	bool NeedsUpload() const	{ return State == EChunkState::PendingUpload; }
	bool NeedsMesh() const		{ return State == EChunkState::PendingMesh; }

	FORCEINLINE float GetDensity(const FVoxelCoord& LocalCoord) const
	{
		if (!LocalCoord.IsValid())
		{
			return 1.0f;	// Out of bounds, treat as air
		}
		return Densities[LocalCoord.ToIndex()];
	}

	FORCEINLINE float GetDensity(int32 X, int32 Y, int32 Z) const
	{
		return GetDensity(FVoxelCoord(X, Y, Z));
	}

	FORCEINLINE void SetDensity(const FVoxelCoord& C, float Value)
	{
		if (C.IsValid())
		{
			Densities[C.ToIndex()] = Value;
		}
	}

	FORCEINLINE void SetDensity(int32 X, int32 Y, int32 Z, float Value)
	{
		SetDensity(FVoxelCoord(X, Y, Z), Value);
	}
	FORCEINLINE float* GetDensityData() { return Densities.GetData(); }
	FORCEINLINE const float* GetDensityData() const { return Densities.GetData(); }

	void FillDensity(float value);
	void UploadMesh(UMaterialInterface* TerrainMaterial);

	UProceduralMeshComponent* GetMeshComponent() const { return MeshComp; }

private:
	TArray<float> Densities;

	UPROPERTY()
	UProceduralMeshComponent* MeshComp = nullptr;

	EChunkState State = EChunkState::Uninitialized;
};
