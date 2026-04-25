#pragma once

#include "Core/EngineAPI.h"
#include "Core/HAL/PlatformTypes.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/String.h"

struct FResourceLoadHistoryEntry
{
    FString AssetType;
    FString AssetPath;
    double  LoadMilliseconds = 0.0;
    bool    bCacheHit = false;
    bool    bSuccess = false;
};

class ENGINE_API UEngineStatics
{
public:
	static uint32 GenUUID();
    static void   RecordResourceLoad(const char* AssetTypeLabel, const FString& AssetPath,
                                     double LoadMilliseconds, bool bCacheHit, bool bSuccess);
    static const TArray<FResourceLoadHistoryEntry>& GetResourceLoadHistory();

	static uint32 NextUUID;
	static uint32 TotalAllocatedBytes;
	static uint32 TotalAllocationCount;
	static float  GridSpacing;

  private:
    static TArray<FResourceLoadHistoryEntry> ResourceLoadHistory;
    static constexpr size_t MaxResourceLoadHistoryCount = 512;
};
