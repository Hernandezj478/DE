#pragma once

#include "CoreMinimal.h"
#include "DEVoxel.generated.h"

USTRUCT(BlueprintType)
struct FDEVoxel
{
	GENERATED_BODY()
public:
	// Block type ID (indexes into CoreData block definitions)
	uint16 BlockTypeID = 0;
	uint8 Rotation = 0;
	
	// Block flags (damaged, wet, powered, etc.)
	uint8 Flags = 0;
	
	FDEVoxel() = default;
	
	explicit FDEVoxel(uint16 InBlockTypeID) : BlockTypeID(InBlockTypeID) {}
};