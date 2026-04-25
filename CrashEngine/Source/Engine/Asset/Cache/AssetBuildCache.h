#pragma once

#include <filesystem>

#include "Asset/Cache/AssetCache.h"
#include "Asset/Cache/AssetCacheTraits.h"
#include "Asset/Source/SourceCache.h"

namespace Asset
{

    class FAssetBuildCache
    {
      public:
        template <typename TTag>
        const FSourceRecord* GetSource(TTag Tag, const std::filesystem::path& Path)
        {
            using TSourceKey = TSourceKeyOf<TTag>;

            auto&                       CacheRef = GetSourceCacheImpl(Tag);
            const std::filesystem::path NormalizedPath = SourceCacheDetail::NormalizePath(Path);
            if (NormalizedPath.empty())
            {
                return nullptr;
            }

            const TSourceKey     SourceKey = TSourceKey::FromPath(NormalizedPath);
            const FSourceRecord* Source = CacheRef.GetOrLoad(SourceKey);
            if (Source == nullptr)
            {
                return nullptr;
            }
            if (!CacheRef.EnsureContentHashLoaded(SourceKey))
            {
                return nullptr;
            }
            return CacheRef.Find(SourceKey);
        }

        template <typename TTag> const FSourceRecord* GetSource(TTag Tag, const FWString& Path)
        {
            return GetSource(Tag, std::filesystem::path(Path));
        }

        template <typename TTag> void InvalidateSource(TTag Tag, const std::filesystem::path& Path)
        {
            using TSourceKey = TSourceKeyOf<TTag>;

            const std::filesystem::path NormalizedPath = SourceCacheDetail::NormalizePath(Path);
            if (NormalizedPath.empty())
            {
                return;
            }

            GetSourceCacheImpl(Tag).Invalidate(TSourceKey::FromPath(NormalizedPath));
        }

        template <typename TTag> void InvalidateSource(TTag Tag, const FWString& Path)
        {
            InvalidateSource(Tag, std::filesystem::path(Path));
        }

        void ClearAll()
        {
            TextureSourceCache.Clear();
            MaterialSourceCache.Clear();
            StaticMeshSourceCache.Clear();
            SubUVAtlasSourceCache.Clear();
            FontAtlasSourceCache.Clear();

            IntermediateTextureCache.Clear();
            IntermediateMaterialCache.Clear();
            IntermediateStaticMeshCache.Clear();
            IntermediateSubUVAtlasCache.Clear();
            IntermediateFontAtlasCache.Clear();

            TextureCookedCache.Clear();
            MaterialCookedCache.Clear();
            StaticMeshCookedCache.Clear();
            SubUVAtlasCookedCache.Clear();
            FontAtlasCookedCache.Clear();
        }

        template <typename TTag>
        TIntermediateCache<TIntermediateKeyOf<TTag>, TIntermediateTypeOf<TTag>>&
        GetIntermediateCache(TTag Tag)
        {
            return GetIntermediateCacheImpl(Tag);
        }

        template <typename TTag>
        TCookedCache<TCookedKeyOf<TTag>, TCookedTypeOf<TTag>>& GetCookedCache(TTag Tag)
        {
            return GetCookedCacheImpl(Tag);
        }

      private:
        TSourceCache<FTextureSourceKey>& GetSourceCacheImpl(FTextureAssetTag)
        {
            return TextureSourceCache;
        }
        TSourceCache<FMaterialSourceKey>& GetSourceCacheImpl(FMaterialAssetTag)
        {
            return MaterialSourceCache;
        }
        TSourceCache<FStaticMeshSourceKey>& GetSourceCacheImpl(FStaticMeshAssetTag)
        {
            return StaticMeshSourceCache;
        }
        TSourceCache<FSubUVAtlasSourceKey>& GetSourceCacheImpl(FSubUVAtlasAssetTag)
        {
            return SubUVAtlasSourceCache;
        }
        TSourceCache<FFontAtlasSourceKey>& GetSourceCacheImpl(FFontAtlasAssetTag)
        {
            return FontAtlasSourceCache;
        }

        TIntermediateCache<FTextureIntermediateKey, FIntermediateTextureData>&
        GetIntermediateCacheImpl(FTextureAssetTag)
        {
            return IntermediateTextureCache;
        }
        TIntermediateCache<FMaterialIntermediateKey, FIntermediateMtlLibraryData>&
        GetIntermediateCacheImpl(FMaterialAssetTag)
        {
            return IntermediateMaterialCache;
        }
        TIntermediateCache<FStaticMeshIntermediateKey, FIntermediateObjData>&
        GetIntermediateCacheImpl(FStaticMeshAssetTag)
        {
            return IntermediateStaticMeshCache;
        }
        TIntermediateCache<FSubUVAtlasIntermediateKey, FIntermediateSubUVAtlasData>&
        GetIntermediateCacheImpl(FSubUVAtlasAssetTag)
        {
            return IntermediateSubUVAtlasCache;
        }
        TIntermediateCache<FFontAtlasIntermediateKey, FIntermediateFontAtlasData>&
        GetIntermediateCacheImpl(FFontAtlasAssetTag)
        {
            return IntermediateFontAtlasCache;
        }

        TCookedCache<FTextureCookedKey, FTextureCookedData>& GetCookedCacheImpl(FTextureAssetTag)
        {
            return TextureCookedCache;
        }
        TCookedCache<FMaterialCookedKey, FMtlCookedLibraryData>&
        GetCookedCacheImpl(FMaterialAssetTag)
        {
            return MaterialCookedCache;
        }
        TCookedCache<FStaticMeshCookedKey, FObjCookedData>& GetCookedCacheImpl(FStaticMeshAssetTag)
        {
            return StaticMeshCookedCache;
        }
        TCookedCache<FSubUVAtlasCookedKey, FSubUVAtlasCookedData>&
        GetCookedCacheImpl(FSubUVAtlasAssetTag)
        {
            return SubUVAtlasCookedCache;
        }
        TCookedCache<FFontAtlasCookedKey, FFontAtlasCookedData>&
        GetCookedCacheImpl(FFontAtlasAssetTag)
        {
            return FontAtlasCookedCache;
        }

      private:
        TSourceCache<FTextureSourceKey>    TextureSourceCache;
        TSourceCache<FMaterialSourceKey>   MaterialSourceCache;
        TSourceCache<FStaticMeshSourceKey> StaticMeshSourceCache;
        TSourceCache<FSubUVAtlasSourceKey> SubUVAtlasSourceCache;
        TSourceCache<FFontAtlasSourceKey>  FontAtlasSourceCache;

        TIntermediateCache<FTextureIntermediateKey, FIntermediateTextureData>
            IntermediateTextureCache;
        TIntermediateCache<FMaterialIntermediateKey, FIntermediateMtlLibraryData>
            IntermediateMaterialCache;
        TIntermediateCache<FStaticMeshIntermediateKey, FIntermediateObjData>
            IntermediateStaticMeshCache;
        TIntermediateCache<FSubUVAtlasIntermediateKey, FIntermediateSubUVAtlasData>
            IntermediateSubUVAtlasCache;
        TIntermediateCache<FFontAtlasIntermediateKey, FIntermediateFontAtlasData>
            IntermediateFontAtlasCache;

        TCookedCache<FTextureCookedKey, FTextureCookedData>       TextureCookedCache;
        TCookedCache<FMaterialCookedKey, FMtlCookedLibraryData>   MaterialCookedCache;
        TCookedCache<FStaticMeshCookedKey, FObjCookedData>        StaticMeshCookedCache;
        TCookedCache<FSubUVAtlasCookedKey, FSubUVAtlasCookedData> SubUVAtlasCookedCache;
        TCookedCache<FFontAtlasCookedKey, FFontAtlasCookedData>   FontAtlasCookedCache;
    };

} // namespace Asset
