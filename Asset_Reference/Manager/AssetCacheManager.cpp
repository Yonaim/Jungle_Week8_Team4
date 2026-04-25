#include "Asset/Manager/AssetCacheManager.h"

#include <filesystem>

#include "Asset/Builder/AssetBuildReport.h"

#include "Engine/Scene/SceneAssetPath.h"

namespace Asset
{
    namespace
    {
        const char* DescribeBuildResultSource(EAssetBuildResultSource Source)
        {
            switch (Source)
            {
            case EAssetBuildResultSource::CookedCache:
                return "cached cooked data";
            case EAssetBuildResultSource::BuiltFromCachedIntermediate:
                return "rebuilt cooked data from cached intermediate";
            case EAssetBuildResultSource::BuiltFromFreshIntermediate:
                return "rebuilt cooked data from newly loaded source";
            default:
                return "unknown";
            }
        }

        void LogBuildReport(const char* AssetType, const FString& Path,
                            const FAssetBuildReport& Report)
        {
            UE_LOG(AssetCache, ELogLevel::Debug,
                   "%s asset ready: %s (%s, cachedIntermediate=%d, cachedCooked=%d, "
                   "builtNewCooked=%d)",
                   AssetType, Path.c_str(), DescribeBuildResultSource(Report.ResultSource),
                   Report.bUsedCachedIntermediate ? 1 : 0, Report.bUsedCachedCooked ? 1 : 0,
                   Report.bBuiltNewCooked ? 1 : 0);
        }
    } // namespace

    FString FAssetCacheManager::StringFromPath(const std::filesystem::path& Path)
    {
        if (Path.empty())
        {
            return {};
        }

        return FPaths::Utf8FromPath(Path);
    }

    std::filesystem::path FAssetCacheManager::ResolveAssetPath(const FString& Path)
    {
        if (Path.empty())
        {
            return {};
        }

        std::filesystem::path Candidate = FPaths::PathFromUtf8(Path);
        if (Candidate.is_absolute())
        {
            return Candidate;
        }

        return Engine::Scene::ResolveSceneAssetPathToAbsolute(Path);
    }

    FAssetCacheManager::FAssetCacheManager()
        : BuildCache(), TextureBuilder(BuildCache), MaterialBuilder(BuildCache),
          StaticMeshBuilder(BuildCache), SubUVAtlasBuilder(BuildCache), FontAtlasBuilder(BuildCache)
    {
    }

    std::shared_ptr<FTextureCookedData>
    FAssetCacheManager::BuildTexture(const FString& Path, const FTextureBuildSettings& Settings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Texture asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(AssetCache, ELogLevel::Debug, "Texture asset path resolved: %s -> %s", Path.c_str(),
               StringFromPath(AbsolutePath).c_str());

        return BuildTextureAbsolute(AbsolutePath, Settings);
    }

    std::shared_ptr<FMtlCookedData> FAssetCacheManager::BuildMaterial(const FString& Path)
    {
        FString LibraryPath = Path;
        FString MaterialName;
        FMaterialBuilder::SplitMaterialAssetPath(Path, LibraryPath, MaterialName);

        const std::filesystem::path AbsolutePath = ResolveAssetPath(LibraryPath);
        if (AbsolutePath.empty())
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Material asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        const FString ResolvedPath =
            MaterialName.empty()
                ? StringFromPath(AbsolutePath)
                : FMaterialBuilder::MakeMaterialAssetPath(AbsolutePath, MaterialName);
        UE_LOG(AssetCache, ELogLevel::Debug, "Material asset path resolved: %s -> %s",
               Path.c_str(), ResolvedPath.c_str());

        return BuildMaterialAbsolute(AbsolutePath, MaterialName);
    }

    std::shared_ptr<FObjCookedData>
    FAssetCacheManager::BuildStaticMesh(const FString&                  Path,
                                        const FStaticMeshBuildSettings& Settings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Static mesh asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(AssetCache, ELogLevel::Debug, "Static mesh asset path resolved: %s -> %s",
               Path.c_str(), StringFromPath(AbsolutePath).c_str());

        return BuildStaticMeshAbsolute(AbsolutePath, Settings);
    }

    std::shared_ptr<FSubUVAtlasCookedData>
    FAssetCacheManager::BuildSubUVAtlas(const FString&               Path,
                                        const FTextureBuildSettings& AtlasTextureSettings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(AssetCache, ELogLevel::Error, "SubUV atlas asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(AssetCache, ELogLevel::Debug, "SubUV atlas asset path resolved: %s -> %s",
               Path.c_str(), StringFromPath(AbsolutePath).c_str());

        return BuildSubUVAtlasAbsolute(AbsolutePath, AtlasTextureSettings);
    }

    std::shared_ptr<FFontAtlasCookedData>
    FAssetCacheManager::BuildFontAtlas(const FString&               Path,
                                       const FTextureBuildSettings& AtlasTextureSettings)
    {
        const std::filesystem::path AbsolutePath = ResolveAssetPath(Path);
        if (AbsolutePath.empty())
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Font atlas asset path resolve failed: %s",
                   Path.c_str());
            return nullptr;
        }

        UE_LOG(AssetCache, ELogLevel::Debug, "Font atlas asset path resolved: %s -> %s",
               Path.c_str(), StringFromPath(AbsolutePath).c_str());

        return BuildFontAtlasAbsolute(AbsolutePath, AtlasTextureSettings);
    }

    std::shared_ptr<FTextureCookedData>
    FAssetCacheManager::BuildTextureAbsolute(const std::filesystem::path& AbsolutePath,
                                             const FTextureBuildSettings& Settings)
    {
        std::shared_ptr<FTextureCookedData> Result = TextureBuilder.Build(AbsolutePath, Settings);

        if (Result)
        {
            UE_LOG(AssetCache, ELogLevel::Debug, "Texture cooked data ready: %s",
                   StringFromPath(AbsolutePath).c_str());
            LogBuildReport("Texture", StringFromPath(AbsolutePath),
                           TextureBuilder.GetLastBuildReport());
        }
        else
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Texture asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    std::shared_ptr<FMtlCookedData>
    FAssetCacheManager::BuildMaterialAbsolute(const std::filesystem::path& AbsolutePath,
                                              const FString&               MaterialName)
    {
        std::shared_ptr<FMtlCookedData> Result =
            MaterialBuilder.BuildMaterial(AbsolutePath, MaterialName);

        const FString LogPath =
            MaterialName.empty()
                ? StringFromPath(AbsolutePath)
                : FMaterialBuilder::MakeMaterialAssetPath(AbsolutePath, MaterialName);
        if (Result)
        {
            UE_LOG(AssetCache, ELogLevel::Debug, "Material cooked data ready: %s",
                   LogPath.c_str());
            LogBuildReport("Material", LogPath, MaterialBuilder.GetLastBuildReport());
        }
        else
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Material asset load failed: %s",
                   LogPath.c_str());
        }

        return Result;
    }

    std::shared_ptr<FObjCookedData>
    FAssetCacheManager::BuildStaticMeshAbsolute(const std::filesystem::path&    AbsolutePath,
                                                const FStaticMeshBuildSettings& Settings)
    {
        std::shared_ptr<FObjCookedData> Result = StaticMeshBuilder.Build(AbsolutePath, Settings);

        if (Result)
        {
            UE_LOG(AssetCache, ELogLevel::Debug, "Static mesh cooked data ready: %s",
                   StringFromPath(AbsolutePath).c_str());
            LogBuildReport("Static mesh", StringFromPath(AbsolutePath),
                           StaticMeshBuilder.GetLastBuildReport());
        }
        else
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Static mesh asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    std::shared_ptr<FSubUVAtlasCookedData>
    FAssetCacheManager::BuildSubUVAtlasAbsolute(const std::filesystem::path& AbsolutePath,
                                                const FTextureBuildSettings& AtlasTextureSettings)
    {
        std::shared_ptr<FSubUVAtlasCookedData> Result =
            SubUVAtlasBuilder.Build(AbsolutePath, AtlasTextureSettings);

        if (Result)
        {
            UE_LOG(AssetCache, ELogLevel::Debug, "SubUV atlas cooked data ready: %s",
                   StringFromPath(AbsolutePath).c_str());
            LogBuildReport("SubUV atlas", StringFromPath(AbsolutePath),
                           SubUVAtlasBuilder.GetLastBuildReport());
        }
        else
        {
            UE_LOG(AssetCache, ELogLevel::Error, "SubUV atlas asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    std::shared_ptr<FFontAtlasCookedData>
    FAssetCacheManager::BuildFontAtlasAbsolute(const std::filesystem::path& AbsolutePath,
                                               const FTextureBuildSettings& AtlasTextureSettings)
    {
        std::shared_ptr<FFontAtlasCookedData> Result =
            FontAtlasBuilder.Build(AbsolutePath, AtlasTextureSettings);

        if (Result)
        {
            UE_LOG(AssetCache, ELogLevel::Debug, "Font atlas cooked data ready: %s",
                   StringFromPath(AbsolutePath).c_str());
            LogBuildReport("Font atlas", StringFromPath(AbsolutePath),
                           FontAtlasBuilder.GetLastBuildReport());
        }
        else
        {
            UE_LOG(AssetCache, ELogLevel::Error, "Font atlas asset load failed: %s",
                   StringFromPath(AbsolutePath).c_str());
        }

        return Result;
    }

    void FAssetCacheManager::ClearAll() { BuildCache.ClearAll(); }

} // namespace Asset
