#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HeightmapProcessor.generated.h"

class UTexture2D;

UCLASS()
class VOXELENGINE_API UHeightmapProcessor : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	float MinHeightVoxels = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	float MaxHeightVoxels = 32.f;

	UHeightmapProcessor();
	~UHeightmapProcessor();

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	bool LoadImage(UTexture2D* Texture);

	UFUNCTION(BlueprintPure, Category = "Heightmap")
	bool IsLoaded() const { return Heights.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Heightmap")
	int32 GetImageWidth() const { return ImageWidth; }

	UFUNCTION(BlueprintPure, Category = "Heightmap")
	int32 GetImageHeight() const { return ImageHeight; }


	UFUNCTION(BlueprintPure, Category = "Heightmap")
	float GetHeightAtPixel(int32 PixelX, int32 PixelY) const;

	float GetHeightAtWorld(float WorldX, float WorldY, float WorldOriginX, float WorldOriginY, float UnitsPerPixel) const;

	float GetHeightAtVoxelCoord(int32 VoxelX, int32 VoxelY) const;

	float GetNormalizedValue(int32 PixelX, int32 PixelY) const;

private:
	TArray<float> Heights;

	int32 ImageWidth = 0;
	int32 ImageHeight = 0;

	float SampleClamped(int32 PixelX, int32 PixelY) const;
	float SampleBiliniear(float FracX, float FracY) const;

};
