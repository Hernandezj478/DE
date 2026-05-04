#pragma once

#include "CoreMinimal.h"
#include "VoxelTypes.generated.h"

// How many cells in cube are formed between 8 neighboring density samples.
static constexpr int32 CHUNK_SIZE = 32;
// Sample per axis, needs one extra for seam row
static constexpr int32 CHUNK_SAMPLE_SIZE = CHUNK_SIZE + 1;
// Total density samples stored per chunk
static constexpr int32 CHUNK_SAMPLE_COUNT = CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE * CHUNK_SAMPLE_SIZE;

// Extended array adds one border voxel on each side of sample grid.
static const int32 CHUNK_EXTENDED_SIZE = CHUNK_SAMPLE_SIZE + 2;

// Physical size of one voxl cell in UU (cm)
static constexpr float VOXEL_SIZE = 50.f;	// Change to 50u after proven concept
// Physical size of one chunk in UU
static constexpr float CHUNK_WORLD_SIZE = CHUNK_SIZE * VOXEL_SIZE;
// Isosurface threshold. Density exactly at this value => on the surface.
// Negative density => solid. Positive density => air
static constexpr float ISO_LEVEL = 0.0f;


USTRUCT(BlueprintType)
struct FVoxelCoord
{
	GENERATED_USTRUCT_BODY()

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

USTRUCT(BlueprintType)
struct FChunkCoord
{
	GENERATED_USTRUCT_BODY()

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
	// Empty
	Air = 0,
	// Surface (1-9)
	Grass		= 1 UMETA(DisplayName = "Grass"),
	Dirt		= 2 UMETA(DisplayName = "Dirt"),
	Sand		= 3 UMETA(DisplayName = "Sand"),
	Gravel		= 4 UMETA(DisplayName = "Gravel"),
	Snow		= 5 UMETA(DisplayNam = "Snow"),

	// Sub-surface (10-19)
	Rock		= 10 UMETA(DisplayName = "Rock"),
	Granite		= 11 UMETA(DisplayName = "Granite"),
	Limestone	= 12 UMETA(DisplayName = "Limestone"),
	Marble		= 13 UMETA(DisplayName = "Marble"),

	// Ores (20-49)
	Iron		= 20 UMETA(DisplayName = "Iron Ore"),
	Copper		= 21 UMETA(DisplayName = "Copper Ore"),
	Tin			= 22 UMETA(DisplayName = "Tin Ore"),
	Lead		= 23 UMETA(DisplayName = "Lead Ore"),
	Coal		= 24 UMETA(DisplayName = "Coal"),
	Titanium	= 25 UMETA(DisplayName = "Titanium Ore"),
	Cobalt		= 26 UMETA(DisplayName = "Cobalt Ore"),

	// Special blocks (200-255)
	Water		= 200 UMETA(DisplayName = "Water"),
	EditorPaint = 253 UMETA(DisplayName = "Editor Paint"),
	Unknown		= 255 UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FVoxelDropEntry
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	FName ItemID = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	int32 MinQuantity = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
	int32 MaxQuantity = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 1.0f;

	FVoxelDropEntry() = default;
};

USTRUCT(BlueprintType)
struct VOXELENGINE_API FVoxelTypeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	int32 TextureArrayIndex = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TSoftObjectPtr<UTexture2D> AlbedoTexture;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TSoftObjectPtr<UTexture2D> NormalTexture;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Textures")
	TSoftObjectPtr<UTexture2D> ORMTexture;

	// Minimum tool tier requiered to mine this voxel (0 = hand)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mining")
	int32 RequiredToolTier = 0;

	// How much damage this voxel absorbeds per mining tick
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mining", meta = (ClampMin = "0.1"))
	float Hardness = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mining")
	bool bIsMineable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mining")
	bool bSupportsStructures = true;

	// Loot
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TArray<FVoxelDropEntry> Drops;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation")
	int32 MinGenerationDepth = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation")
	int32 MaxGenerationDepth = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OreRarityThreshold = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation", meta = (ClampMin = "0.001"))
	float OreNoiseFrequency = 0.05f;

	// If non-empty, this ore only appears in these biomes.
	// TODO: Change from FName to biome enum. this will allow to select biome from dropdown
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation")
	TArray<FName> AllowedBiomes;

	FVoxelTypeData() = default;
};

UENUM(BlueprintType)
enum class EVoxelOpType : uint8
{

	DigSphere = 0	UMETA(DisplayName = "Dig Sphere"),
	AddSphere = 1	UMETA(DisplayName = "Add Sphere"),
	PaintType = 2	UMETA(DisplayName = "Paint Type")

};

USTRUCT(BlueprintType)
struct FVoxelModification
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	EVoxelOpType OpType = EVoxelOpType::DigSphere;

	UPROPERTY()
	FVector WorldCenter = FVector::ZeroVector;

	UPROPERTY()
	float Radius = 1.0f;

	UPROPERTY()
	float Strength = 1.0f;

	UPROPERTY()
	uint8 TypeToPaint = static_cast<uint8>(EVoxelType::Unknown);

	FVoxelModification() = default;

	FVoxelModification(EVoxelOpType InOp, FVector InCenter, float InRadius, float InStrength = 1.0f)
		: OpType(InOp), WorldCenter(InCenter), Radius(InRadius), Strength(InStrength)
	{}
};
// NOTE: we should use this struct for the voxel type data
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