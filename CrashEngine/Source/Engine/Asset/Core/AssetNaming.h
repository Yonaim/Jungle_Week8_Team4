#pragma once

#include "Core/CoreMinimal.h"
#include "Core/EngineAPI.h"

namespace Asset
{
    enum class EAssetFileKind : uint8
    {
        Unknown = 0,
        Scene,
        Texture,
        Font,
        TextureAtlas,
        StaticMesh,
        MaterialLibrary,
    };

    ENGINE_API bool           AssetPathEndsWith(const FString& Path, const char* Suffix);
    ENGINE_API EAssetFileKind ClassifyAssetPath(const FString& Path);
    ENGINE_API const char*    GetAssetFileKindLabel(EAssetFileKind Kind);

    ENGINE_API bool HasSourceExtension(EAssetFileKind Kind, const FString& Path);
    ENGINE_API bool HasBakedExtension(EAssetFileKind Kind, const FString& Path);

    ENGINE_API FString GetPrimarySourceExtension(EAssetFileKind Kind);
    ENGINE_API FString GetPrimaryBakedExtension(EAssetFileKind Kind);

    ENGINE_API FString MakeBakedAssetPath(const FString& SourcePath);
} // namespace Asset
