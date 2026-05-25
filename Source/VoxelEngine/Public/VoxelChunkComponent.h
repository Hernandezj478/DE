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
	DensityReady,
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

	FORCEINLINE const FChunkCoord& GetChunkCoord() const 
	{
		return ChunkCoord; 
	}
	FORCEINLINE FVector GetChunkWorldOrigin() const 
	{
		return ChunkCoord.ToWorldPosition(); 
	}

	void SetState(EChunkState NewState){ State = NewState; }
	void MarkDirty()				   { State = EChunkState::Dirty; }

	UFUNCTION(BlueprintPure, Category = "Voxel Chunk")
	EChunkState GetState() const
	{ 
		return State; 
	}
	bool IsReady() const		
	{
		return State == EChunkState::Ready; 
	}
	bool IsDirty() const		
	{ 
		return State == EChunkState::Dirty; 
	}
	bool IsGenerating() const	
	{ 
		return 
			State == EChunkState::GeneratingDensity 
		 || State == EChunkState::GeneratingMesh; 
	}
	bool NeedsUpload() const	
	{ 
		return State == EChunkState::PendingUpload; 
	}
	bool NeedsMesh() const		
	{
		return State == EChunkState::PendingMesh; 
	}
	bool HasDensityData() const 
	{ 
		return static_cast<int>(State) > static_cast<int>(EChunkState::GeneratingDensity); 
	}

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
	FORCEINLINE float* GetDensityData() 
	{ 
		return Densities.GetData(); 
	}
	FORCEINLINE const float* GetDensityData() const 
	{ 
		return Densities.GetData(); 
	}

	FORCEINLINE EVoxelType GetVoxelType(const FVoxelCoord& C) const
	{
		if (!C.IsValid())
		{
			return EVoxelType::Air;
		}
		return static_cast<EVoxelType>(VoxelTypes[C.ToIndex()]);
	}

	FORCEINLINE EVoxelType GetVoxelType(int32 X, int32 Y, int32 Z) const
	{
		return GetVoxelType(FVoxelCoord(X, Y, Z));
	}

	FORCEINLINE void SetVoxelType(const FVoxelCoord& C, EVoxelType Type)
	{
		if (C.IsValid())
		{
			VoxelTypes[C.ToIndex()] = static_cast<uint8>(Type);
		}
	}

	FORCEINLINE void SetVoxelType(int32 X, int32 Y, int32 Z, EVoxelType Type)
	{
		SetVoxelType(FVoxelCoord(X, Y, Z), Type);
	}

	FORCEINLINE const uint8* GetVoxelTypeData() const { return VoxelTypes.GetData(); }

	void ApplyGeneratedData(TArray<float>&& InDensities, TArray<uint8>&& InVoxelTypes);
	void ClearMesh();
	void FillDensity(float value);
	void UploadMesh(UMaterialInterface* TerrainMaterial);

	UProceduralMeshComponent* GetMeshComponent() const { return MeshComp; }

private:
	TArray<float> Densities;
	TArray<uint8> VoxelTypes;

	UPROPERTY()
	UProceduralMeshComponent* MeshComp = nullptr;

	EChunkState State = EChunkState::Uninitialized;
};
