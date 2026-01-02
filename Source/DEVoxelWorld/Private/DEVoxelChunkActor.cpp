// Fill out your copyright notice in the Description page of Project Settings.


#include "DEVoxelChunkActor.h"


ADEVoxelChunkActor::ADEVoxelChunkActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ADEVoxelChunkActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADEVoxelChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADEVoxelChunkActor::InitializeChunk(const FIntVector& InChunkCoords)
{
	ChunkCoords = InChunkCoords;
	
	// Position actor in world space
	constexpr int32 BlockSize = 100; // 1m = 100 units
	constexpr int32 ChunkSize = 64;
	
	FVector WorldLocation = FVector(
		InChunkCoords.X * ChunkSize * BlockSize,
		InChunkCoords.Y * ChunkSize * BlockSize,
		InChunkCoords.Z * ChunkSize * BlockSize
	);
	
	SetActorLocation(WorldLocation);
}

