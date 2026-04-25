#pragma once

#include <filesystem>
#include <memory>

#include "Asset/Builder/AssetBuildReport.h"

#include "Asset/Cache/AssetBuildCache.h"
#include "Asset/Builder/TextureBuilder.h"
#include "Asset/Cache/BuildSettings.h"
#include "Asset/Cooked/FontAtlasCookedData.h"
#include "Asset/Intermediate/IntermediateFontAtlasData.h"

namespace Asset
{

    class FFontAtlasBuilder
    {
      public:
        explicit FFontAtlasBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

        std::shared_ptr<FFontAtlasCookedData>
        Build(const std::filesystem::path& Path,
              const FTextureBuildSettings& AtlasTextureSettings = {});
        std::shared_ptr<FFontAtlasCookedData>
        Build(const FWString& Path, const FTextureBuildSettings& AtlasTextureSettings = {})
        {
            return Build(std::filesystem::path(Path), AtlasTextureSettings);
        }

        const FAssetBuildReport& GetLastBuildReport() const { return LastBuildReport; }

      private:
        std::shared_ptr<FIntermediateFontAtlasData> ParseFontAtlas(const FSourceRecord& Source);
        std::shared_ptr<FFontAtlasCookedData>
        CookFontAtlas(const FSourceRecord& Source, const FIntermediateFontAtlasData& Intermediate,
                      const FTextureBuildSettings& AtlasTextureSettings);

        static bool     ReadAllText(const std::filesystem::path& Path, FString& OutText);
        static FString  Trim(const FString& Value);
        static FString  ExtractQuotedValue(const FString& Line);
        static bool     TryParseKeyValueLine(const FString& Line, TMap<FString, FString>& OutPairs);
        static FWString ResolveRelativePath(const std::filesystem::path& BasePath,
                                            const FString&               RelativePath);

      private:
        FAssetBuildCache& Cache;
        FAssetBuildReport LastBuildReport;
    };

} // namespace Asset
