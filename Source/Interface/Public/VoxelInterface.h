

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "VoxelInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVoxelInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AVoxelWorldActor;
class INTERFACE_API IVoxelInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void RequestTerrainDig(AVoxelWorldActor* TerrainActot, FVector WorldCenter, float Radius, float Strength) = 0;
	virtual void RequestTerrainAdd(AVoxelWorldActor* TerrainActot, FVector WorldCenter, float Radius, float Strength) = 0;
	
};
