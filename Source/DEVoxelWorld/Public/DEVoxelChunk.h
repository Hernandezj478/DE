#pragma once

#include "CoreMinimal.h"
#include "DEVoxel.h"
#include "DEVoxelChunk.generated.h"

static constexpr int32 DE_CHUNK_SIZE = 64;
static constexpr int32 DE_CHUNK_VOLUME = DE_CHUNK_SIZE * DE_CHUNK_SIZE * DE_CHUNK_SIZE;

USTRUCT()
struct FDEVoxelChunk
{
	GENERATED_BODY()
public:
	// World-space chunk coordinate
	UPROPERTY()
	FIntVector ChunkCoords = FIntVector::ZeroValue;
	
	//Fixed size voxel storage
	TArray<FDEVoxel> Voxels;
	
	FDEVoxelChunk()
	{
		Voxels.SetNum(DE_CHUNK_VOLUME);
	}
	
	FORCEINLINE int32 ToIndex(int32 X, int32 Y, int32 Z) const
	{
		return X + (Y * DE_CHUNK_SIZE) + (Z * DE_CHUNK_SIZE * DE_CHUNK_SIZE);
	}
	
	FORCEINLINE FDEVoxel& GetVoxel(int32 X, int32 Y, int32 Z)
	{
		return Voxels[ToIndex(X, Y, Z)];
	}
	
	FORCEINLINE const FDEVoxel& GetVoxel(int32 X, int32 Y, int32 Z) const
	{
		return Voxels[ToIndex(X, Y, Z)];
	}
	
	FORCEINLINE void SetChunkCoords(const FIntVector& InChunkCoords)
	{
		ChunkCoords = InChunkCoords;
	}
};