#pragma once

#include "CoreMinimal.h"

// How many cells in cube are formed between 8 neighboring density samples.
static constexpr int32 CHUNK_SIZE = 32;
// Sample per axis, needs one extra for seam row
static constexpr int32 CHUNK_SAMPLE_SIZE = CHUNK_SIZE + 1;
// Total density samples stored per chunk
static constexpr int32 CHUNK_SAMPLE_COUNT = CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE;

// Physical size of one voxl cell in UU (cm)
static constexpr float VOXEL_SIZE = 100.f;	// Change to 50u after proven concept
// Physical size of one chunk in UU
static constexpr float CHUNK_WORLD_SIZE = CHUNK_SIZE * VOXEL_SIZE;
// Isosurface threshold. Density exactly at this value => on the surface.
// Negative density => solid. Positive density => air
static constexpr float ISO_LEVEL = 0.0f;

struct FVoxelCoord
{
	int32 X = 0;
	int32 Y = 0;
	int32 Z = 0;

	FVoxelCoord() = default;
	FVoxelCoord(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

	FORCEINLINE int32 ToIndex() const
	{
		return Z * (CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE)
			+ Y * CHUNK_SAMPLE_SIZE
			+ X;
	}
	static FORCEINLINE FVoxelCoord FromIndex(int32 Index)
	{
		const int32 Z = Index / (CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE);
		const int32 Rem = Index % (CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE);
		return FVoxelCoord(Rem % CHUNK_SAMPLE_SIZE, Rem / CHUNK_SAMPLE_SIZE, Z);
	}
	FORCEINLINE bool IsValid() const
	{
		return X >= 0 && X < CHUNK_SAMPLE_SIZE
			&& Y >= 0 && Y < CHUNK_SAMPLE_SIZE
			&& Z >= 0 && Z < CHUNK_SAMPLE_SIZE;
	}

	FORCEINLINE FVoxelCoord operator+(const FVoxelCoord& Other) const
	{
		return FVoxelCoord(X + Other.X, Y + Other.Y, Z + Other.Z);
	}

	bool operator==(const FVoxelCoord& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
};

struct FChunkCoord
{
	int32 X = 0;
	int32 Y = 0;
	int32 Z = 0;

	FChunkCoord() = default;
	FChunkCoord(int32 InX, int32 InY, int32 InZ) : X(InX), Y(InY), Z(InZ) {}

	FORCEINLINE FVector ToWorldPosition() const
	{
		return FVector(
			X * CHUNK_WORLD_SIZE,
			Y * CHUNK_WORLD_SIZE,
			Z * CHUNK_WORLD_SIZE);
	}

	static FORCEINLINE FChunkCoord FromWorldPosition(const FVector& WorldPos)
	{
		return FChunkCoord(
			FMath::FloorToInt(WorldPos.X / CHUNK_WORLD_SIZE),
			FMath::FloorToInt(WorldPos.Y / CHUNK_WORLD_SIZE),
			FMath::FloorToInt(WorldPos.Z / CHUNK_WORLD_SIZE)
		);
	}

	FORCEINLINE FChunkCoord operator+(const FChunkCoord& Other) const
	{
		return FChunkCoord(X + Other.X, Y + Other.Y, Z + Other.Z);
	}

	bool operator==(const FChunkCoord& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
};

FORCEINLINE uint32 GetTypeHash(const FChunkCoord& Coord)
{
	uint32 H = HashCombine(GetTypeHash(Coord.X), GetTypeHash(Coord.Y));
	return HashCombine(H, GetTypeHash(Coord.Z));
}

UENUM(BlueprintType)
enum class EVoxelType : uint8
{
	Air = 0,	// Empty - no geometry generated
	Stone = 1,
	Dirt = 2,
	Wood = 3
	// Add more as needed - max 255 types
};

struct FVoxelMaterial
{
	int32 MaterialIndex = 0;
	float BlendWeight = 1.0f;

	FVoxelMaterial() = default;
	FVoxelMaterial(int32 InMaterialIndex) : MaterialIndex(InMaterialIndex) {}
};

struct FHermiteData
{
	float T = 0.f;

	FVector Normal = FVector::ZeroVector;
	bool bHasIntersection = false;
};