#pragma once

#include <filesystem>

#include "Asset/Serialization/CookedDataSerialization.h"
#include "Asset/Serialization/WindowsBinArchive.h"

namespace Asset::Binary
{
    enum class ECookedBinaryAssetType : uint32
    {
        Unknown = 0,
        Texture,
        MaterialLibrary,
        StaticMesh,
        FontAtlas,
        SubUVAtlas,
    };

    ENGINE_API FString GetSourceExtension(ECookedBinaryAssetType AssetType);
    ENGINE_API FString GetBinaryExtension(ECookedBinaryAssetType AssetType);
    ENGINE_API bool    TryGetAssetTypeFromSourcePath(const FString&          SourcePath,
                                                     ECookedBinaryAssetType& OutType);
    ENGINE_API FString MakeBinaryPathFromSourcePath(const FString& SourcePath);

    ENGINE_API bool SaveTexture(const FTextureCookedData& Data, const FString& Path);
    ENGINE_API bool SaveTexture(const FTextureCookedData& Data, const std::filesystem::path& Path);
    ENGINE_API bool LoadTexture(const FString& Path, FTextureCookedData& OutData);
    ENGINE_API bool LoadTexture(const std::filesystem::path& Path, FTextureCookedData& OutData);

    ENGINE_API bool SaveMaterialLibrary(const FMtlCookedLibraryData& Data, const FString& Path);
    ENGINE_API bool SaveMaterialLibrary(const FMtlCookedLibraryData&       Data,
                                        const std::filesystem::path& Path);
    ENGINE_API bool LoadMaterialLibrary(const FString& Path, FMtlCookedLibraryData& OutData);
    ENGINE_API bool LoadMaterialLibrary(const std::filesystem::path& Path, FMtlCookedLibraryData& OutData);

    ENGINE_API bool SaveStaticMesh(const FObjCookedData& Data, const FString& Path);
    ENGINE_API bool SaveStaticMesh(const FObjCookedData& Data, const std::filesystem::path& Path);
    ENGINE_API bool LoadStaticMesh(const FString& Path, FObjCookedData& OutData);
    ENGINE_API bool LoadStaticMesh(const std::filesystem::path& Path, FObjCookedData& OutData);

    ENGINE_API bool SaveFontAtlas(const FFontAtlasCookedData& Data, const FString& Path);
    ENGINE_API bool SaveFontAtlas(const FFontAtlasCookedData& Data, const std::filesystem::path& Path);
    ENGINE_API bool LoadFontAtlas(const FString& Path, FFontAtlasCookedData& OutData);
    ENGINE_API bool LoadFontAtlas(const std::filesystem::path& Path, FFontAtlasCookedData& OutData);

    ENGINE_API bool SaveSubUVAtlas(const FSubUVAtlasCookedData& Data, const FString& Path);
    ENGINE_API bool SaveSubUVAtlas(const FSubUVAtlasCookedData& Data, const std::filesystem::path& Path);
    ENGINE_API bool LoadSubUVAtlas(const FString& Path, FSubUVAtlasCookedData& OutData);
    ENGINE_API bool LoadSubUVAtlas(const std::filesystem::path& Path, FSubUVAtlasCookedData& OutData);

} // namespace Asset::Binary
