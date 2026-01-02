// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DEVoxelChunk.h"
#include "DEVoxelChunkActor.generated.h"

UCLASS()
class DEVOXELWORLD_API ADEVoxelChunkActor : public AActor
{
	GENERATED_BODY()

public:
	
	ADEVoxelChunkActor();
	virtual void Tick(float DeltaTime) override;

	// Initialize chunk with world chunk coordinates
	void InitializeChunk(const FIntVector& InChunkCoords);
	
	// Access the chunk data
	FORCEINLINE FDEVoxelChunk& GetChunk() {return Chunk; }
	FORCEINLINE const FDEVoxelChunk& GetChunk() const {return Chunk; }
	
	// Get Chunk coordinates
	FORCEINLINE const FIntVector& GetChunkCoords() const {return ChunkCoords; }
	
protected:
	virtual void BeginPlay() override;

public:
	// Chunk grid coordinates
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	FIntVector ChunkCoords = FIntVector::ZeroValue;
	
	// Runtime voxel data
	FDEVoxelChunk Chunk;
};
