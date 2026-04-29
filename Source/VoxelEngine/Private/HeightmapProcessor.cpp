#include "HeightmapProcessor.h"
#include "Engine/Texture2D.h"
#include "Logger.h"

UHeightmapProcessor::UHeightmapProcessor()
{
}

UHeightmapProcessor::~UHeightmapProcessor()
{
}

bool UHeightmapProcessor::LoadImage(UTexture2D* Texture)
{
	Heights.Empty();
	ImageWidth = 0;
	ImageHeight = 0;
	if (!Texture)
	{
		LOG_ERROR(LogVoxelEngine, "No Texture loaded");
		return false;
	}
	const FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	const void* Data = Mip.BulkData.LockReadOnly();
	if (!Data)
	{
		Mip.BulkData.Unlock();
		LOG_ERROR(LogVoxelEngine, "Could not lock texture mip. Make sure texture has Allow CPU Access is enabled");
		return false;
	}
	ImageWidth = Mip.SizeX;
	ImageHeight = Mip.SizeY;
	Heights.SetNumUninitialized(ImageWidth * ImageHeight);
	const EPixelFormat PixelFormat = Texture->GetPixelFormat();
	if (PixelFormat == PF_G8)
	{
		const uint8* Pixels = static_cast<const uint8*>(Data);
		for (int32 i = 0; i < ImageWidth * ImageHeight; i++)
		{
			Heights[i] = Pixels[i] / 255.f;
		}
	}
	else if (PixelFormat == PF_B8G8R8A8)
	{
		const uint8* Pixels = static_cast<const uint8*>(Data);
		for (int32 i = 0; i < ImageWidth * ImageHeight; i++)
		{
			const uint8 B = Pixels[i * 4];
			const uint8 G = Pixels[i * 4 + 1];
			const uint8 R = Pixels[i * 4 + 2];
			Heights[i] = (0.299f * R + 0.587f * G + 0.114f * B) / 255.0f;
		}
	}
	else if (PixelFormat == PF_G16)
	{
		const uint16* Pixels = static_cast<const uint16*>(Data);
		for (int32 i = 0; i < ImageWidth * ImageHeight; i++)
		{
			Heights[i] = Pixels[i] / 65535.f;
		}
	}
	else
	{
		Mip.BulkData.Unlock();
		LOG_ERROR(LogVoxelEngine, "Unsupported pixel format %d. Use TC_Grayscale (PF_G8), PF_G16, or TC_Default (PF_B8G8R8A8).",
			(int32)PixelFormat);
		return false;
	}
	Mip.BulkData.Unlock();
	LOG_DEBUG(LogVoxelEngine, "Loaded texture WxH: %dx%d, Format: %d, Height Range: %.1f-%.1f voxels",
		ImageWidth, ImageHeight, (int32)PixelFormat, MinHeightVoxels, MaxHeightVoxels);
	return true;
}

float UHeightmapProcessor::GetHeightAtPixel(int32 PixelX, int32 PixelY) const
{
	if (!IsLoaded())
	{
		return MinHeightVoxels;
	}
	const float T = SampleClamped(PixelX, PixelY);
	return FMath::Lerp(MinHeightVoxels, MaxHeightVoxels, T);
}

float UHeightmapProcessor::GetHeightAtWorld(float WorldX, float WorldY, float WorldOriginX, float WorldOriginY, float UnitsPerPixel) const
{
	if (!IsLoaded() || UnitsPerPixel <= 0.f)
	{
		return MinHeightVoxels;
	}
	const float FracX = (WorldX - WorldOriginX) / UnitsPerPixel;
	const float FracY = (WorldY - WorldOriginY) / UnitsPerPixel;
	
	const float T = SampleBiliniear(FracX, FracY);
	return FMath::Lerp(MinHeightVoxels, MaxHeightVoxels, T);
}

float UHeightmapProcessor::GetHeightAtVoxelCoord(int32 VoxelX, int32 VoxelY) const
{
	return GetHeightAtPixel(VoxelX, VoxelY);
}

float UHeightmapProcessor::GetNormalizedValue(int32 PixelX, int32 PixelY) const
{
	if (!IsLoaded())
	{
		return 0.f;
	}
	return SampleClamped(PixelX, PixelY);
}

float UHeightmapProcessor::SampleClamped(int32 PixelX, int32 PixelY) const
{
	PixelX = FMath::Clamp(PixelX, 0, ImageWidth - 1);
	PixelY = FMath::Clamp(PixelY, 0, ImageHeight - 1);
	return Heights[PixelY * ImageWidth + PixelX];
}

float UHeightmapProcessor::SampleBiliniear(float FracX, float FracY) const
{
	const int32 X0 = (int32)FracX;
	const int32 Y0 = (int32)FracY;

	const float TX = FracX - X0;
	const float TY = FracY - Y0;

	const float S00 = SampleClamped(X0, Y0);
	const float S10 = SampleClamped(X0 + 1, Y0);
	const float S01 = SampleClamped(X0, Y0 + 1);
	const float S11 = SampleClamped(X0 + 1, Y0 + 1);
	return FMath::Lerp(FMath::Lerp(S00, S10, TX), FMath::Lerp(S01, S11, TX), TY);
}
