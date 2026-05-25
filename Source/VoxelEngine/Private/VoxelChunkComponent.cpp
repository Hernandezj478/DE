#include "VoxelChunkComponent.h"
#include "ProceduralMeshComponent.h"
#include "Logger.h"

UVoxelChunkComponent::UVoxelChunkComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	Densities.SetNumZeroed(CHUNK_SAMPLE_COUNT);
	VoxelTypes.SetNumZeroed(CHUNK_SAMPLE_COUNT);
}

// Called when the game starts
void UVoxelChunkComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVoxelChunkComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UVoxelChunkComponent::ApplyGeneratedData(TArray<float>&& InDensities, TArray<uint8>&& InVoxelTypes)
{
	if (InDensities.Num() != CHUNK_SAMPLE_COUNT || InVoxelTypes.Num() != CHUNK_SAMPLE_COUNT)
	{
		LOG_ERROR(LogVoxelEngine, "Invalid array size. Check array size for Densities or VoxelTypes");
		return;
	}
	Densities = MoveTemp(InDensities);
	VoxelTypes = MoveTemp(InVoxelTypes);
}

void UVoxelChunkComponent::ClearMesh()
{
	if (MeshComp)
	{
		MeshComp->ClearAllMeshSections();
		MeshComp->SetCanEverAffectNavigation(false);
	}
	State = EChunkState::DensityReady;
}

void UVoxelChunkComponent::FillDensity(float Value)
{
	for (float& D : Densities)
	{
		D = Value;
	}
}

void UVoxelChunkComponent::UploadMesh(UMaterialInterface* TerrainMaterial)
{
	if(!MeshComp)
	{
		const FName CompName = FName(*FString::Printf(
			TEXT("ChunkMesh_%d_%d_%d"),
			ChunkCoord.X, ChunkCoord.Y, ChunkCoord.Z));
		AActor* Owner = GetOwner();
		check(Owner);

		MeshComp = NewObject<UProceduralMeshComponent>(Owner, CompName);
		MeshComp->SetNetAddressable();
		MeshComp->SetCanEverAffectNavigation(false);
		MeshComp->bUseAsyncCooking = false;
		MeshComp->bUseComplexAsSimpleCollision = true;

		MeshComp->SetupAttachment(Owner->GetRootComponent());
		MeshComp->RegisterComponent();
		const FVector RelativePos = ChunkCoord.ToWorldPosition() - Owner->GetActorLocation();
		MeshComp->SetRelativeLocation(RelativePos);
	}
	FChunkMeshData LocalData;
	{
		FScopeLock Lock(&MeshDataMutex);
		LocalData = MoveTemp(PendingMeshData);
	}
	if(LocalData.IsEmpty())
	{
		MeshComp->ClearMeshSection(0);
		MeshComp->SetCanEverAffectNavigation(false);
		State = EChunkState::Ready;
		return;
	}
	MeshComp->CreateMeshSection(
		0,
		LocalData.Vertices,
		LocalData.Triangles,
		LocalData.Normals,
		LocalData.UVs,
		LocalData.VertexColors,
		LocalData.Tangents,
		true);
	MeshComp->RecreatePhysicsState();
	MeshComp->SetCanEverAffectNavigation(true);

	if (TerrainMaterial)
	{
		MeshComp->SetMaterial(0, TerrainMaterial);
	}
	State = EChunkState::Ready;
}
