#pragma once

#include "CoreMinimal.h"
#include "VoxelTypes.h"
#include "ProceduralMeshComponent.h"
struct FChunkMeshData
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;

    TMap<FIntVector, int32> VertexCache;

    void Reset()
    {
        Vertices.Reset();
        Triangles.Reset();
        Normals.Reset();
        UVs.Reset();
        VertexColors.Reset();
        Tangents.Reset();
        VertexCache.Reset();
    }

    bool IsEmpty() const { return Vertices.Num() == 0; }
};

class UVoxelChunkComponent;
class VOXELENGINE_API FMarchingCubeMesher
{
public:
    static void MeshChunk(const UVoxelChunkComponent& Chunk, FChunkMeshData& OutMeshData);

private:
    static void ProcessCell(const UVoxelChunkComponent& Chunk, int32 CX, int32 CY, int32 CZ, FChunkMeshData& OutMeshData);
    static FVector ComputeGradient(const UVoxelChunkComponent& Chunk, int32 X, int32 Y, int32 Z);
	static FVector InterpolateVertex(const FVector& P1, float D1, const FVector& P2, float D2);

    static const FVoxelCoord CornerOffsets[8];
    static const int32 EdgeTable[256];
    static const int32 TriTable[256][16];

};
