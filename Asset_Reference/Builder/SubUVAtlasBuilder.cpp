#include "Asset/Builder/SubUVAtlasBuilder.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "ThirdParty/nlohmann/json.hpp"

#include "Asset/Cache/AssetKeyUtils.h"
#include "Asset/Core/AssetNaming.h"
#include "Asset/Serialization/CookedDataBinaryIO.h"
#include "Core/Misc/Paths.h"

namespace Asset
{
    std::shared_ptr<FSubUVAtlasCookedData>
    FSubUVAtlasBuilder::Build(const std::filesystem::path& Path,
                              const FTextureBuildSettings& AtlasTextureSettings)
    {
        LastBuildReport.Reset();
        const FSourceRecord* Source = Cache.GetSource(FSubUVAtlasAssetTag{}, Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        const FString BakedAtlasPath = MakeBakedAssetPath(FPaths::Utf8FromPath(Source->NormalizedPath));
        if (!BakedAtlasPath.empty())
        {
            FSubUVAtlasCookedData BakedData;
            if (Binary::LoadSubUVAtlas(BakedAtlasPath, BakedData) && BakedData.IsValid())
            {
                LastBuildReport.bUsedCachedCooked = true;
                LastBuildReport.ResultSource = EAssetBuildResultSource::CookedCache;
                return std::make_shared<FSubUVAtlasCookedData>(std::move(BakedData));
            }
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FSubUVAtlasAssetTag{});
        std::shared_ptr<FIntermediateSubUVAtlasData> Intermediate = ParseAtlas(*Source);
        if (!Intermediate)
        {
            return nullptr;
        }

        const FSubUVAtlasIntermediateKey IntermediateKey =
            KeyUtils::MakeIntermediateKey(*Intermediate);
        std::shared_ptr<FIntermediateSubUVAtlasData> CachedIntermediate =
            IntermediateCache.Find(IntermediateKey);
        if (CachedIntermediate)
        {
            Intermediate = CachedIntermediate;
            LastBuildReport.bUsedCachedIntermediate = true;
        }
        else
        {
            IntermediateCache.Insert(IntermediateKey, Intermediate);
        }

        const FSubUVAtlasCookedKey CookedKey =
            KeyUtils::MakeCookedKey(IntermediateKey, AtlasTextureSettings);

        auto& CookedCache = Cache.GetCookedCache(FSubUVAtlasAssetTag{});
        std::shared_ptr<FSubUVAtlasCookedData> Cooked = CookedCache.Find(CookedKey);
        if (!Cooked)
        {
            LastBuildReport.bBuiltNewCooked = true;
            Cooked = CookAtlas(*Source, *Intermediate, AtlasTextureSettings);
            if (!Cooked)
            {
                return nullptr;
            }
            CookedCache.Insert(CookedKey, Cooked);
        }
        else
        {
            LastBuildReport.bUsedCachedCooked = true;
        }

        if (LastBuildReport.bUsedCachedCooked)
        {
            LastBuildReport.ResultSource = EAssetBuildResultSource::CookedCache;
        }
        else if (LastBuildReport.bUsedCachedIntermediate)
        {
            LastBuildReport.ResultSource = EAssetBuildResultSource::BuiltFromCachedIntermediate;
        }
        else if (Cooked)
        {
            LastBuildReport.ResultSource = EAssetBuildResultSource::BuiltFromFreshIntermediate;
        }

        if (Cooked && !BakedAtlasPath.empty())
        {
            Binary::SaveSubUVAtlas(*Cooked, BakedAtlasPath);
        }

        return Cooked;
    }

    std::shared_ptr<FIntermediateSubUVAtlasData>
    FSubUVAtlasBuilder::ParseAtlas(const FSourceRecord& Source)
    {
        FString Text;
        if (!ReadAllText(Source.NormalizedPath, Text))
        {
            return nullptr;
        }

        nlohmann::json Root;
        try
        {
            Root = nlohmann::json::parse(Text);
        }
        catch (const std::exception&)
        {
            return nullptr;
        }

        auto Result = std::make_shared<FIntermediateSubUVAtlasData>();
        Result->Info.Name = FPaths::Utf8FromPath(Source.NormalizedPath.stem());

        if (Root.contains("meta") && Root["meta"].is_object())
        {
            const auto& Meta = Root["meta"];

            if (Meta.contains("image") && Meta["image"].is_string())
            {
                Result->AtlasImagePath =
                    ResolveRelativePath(Source.NormalizedPath, Meta["image"].get<FString>());
            }
            else if (Meta.contains("atlasImagePath") && Meta["atlasImagePath"].is_string())
            {
                Result->AtlasImagePath =
                    ResolveRelativePath(Source.NormalizedPath, Meta["atlasImagePath"].get<FString>());
            }
            else if (Meta.contains("texture") && Meta["texture"].is_string())
            {
                Result->AtlasImagePath =
                    ResolveRelativePath(Source.NormalizedPath, Meta["texture"].get<FString>());
            }

            if (Meta.contains("app") && Meta["app"].is_string())
            {
                Result->Info.Name = Meta["app"].get<FString>();
            }
        }

        if (Root.contains("name") && Root["name"].is_string())
        {
            Result->Info.Name = Root["name"].get<FString>();
        }

        if (Root.contains("fps") && Root["fps"].is_number())
        {
            Result->Info.FPS = Root["fps"].get<float>();
        }

        if (Root.contains("loop") && Root["loop"].is_boolean())
        {
            Result->Info.bLoop = Root["loop"].get<bool>();
        }

        if (Root.contains("frameWidth") && Root["frameWidth"].is_number_unsigned())
        {
            Result->Info.FrameWidth = Root["frameWidth"].get<uint32>();
        }

        if (Root.contains("frameHeight") && Root["frameHeight"].is_number_unsigned())
        {
            Result->Info.FrameHeight = Root["frameHeight"].get<uint32>();
        }

        if (Root.contains("columns") && Root["columns"].is_number_unsigned())
        {
            Result->Info.Columns = Root["columns"].get<uint32>();
        }

        if (Root.contains("rows") && Root["rows"].is_number_unsigned())
        {
            Result->Info.Rows = Root["rows"].get<uint32>();
        }

        if (Root.contains("frameCount") && Root["frameCount"].is_number_unsigned())
        {
            Result->Info.FrameCount = Root["frameCount"].get<uint32>();
        }

        if (Root.contains("frames") && Root["frames"].is_object())
        {
            uint32 RunningId = 0;

            for (auto It = Root["frames"].begin(); It != Root["frames"].end(); ++It)
            {
                const FString FrameName = It.key();
                const auto&   FrameObject = It.value();

                if (!FrameObject.is_object() || !FrameObject.contains("frame") ||
                    !FrameObject["frame"].is_object())
                {
                    continue;
                }

                const auto& Rect = FrameObject["frame"];

                if (!Rect.contains("x") || !Rect.contains("y") ||
                    !Rect.contains("w") || !Rect.contains("h"))
                {
                    continue;
                }

                FSubUVFrame Frame;
                Frame.Id = RunningId++;
                Frame.X = Rect["x"].get<uint32>();
                Frame.Y = Rect["y"].get<uint32>();
                Frame.Width = Rect["w"].get<uint32>();
                Frame.Height = Rect["h"].get<uint32>();

                if (FrameObject.contains("pivot") && FrameObject["pivot"].is_object())
                {
                    const auto& Pivot = FrameObject["pivot"];
                    if (Pivot.contains("x") && Pivot["x"].is_number())
                    {
                        Frame.PivotX = Pivot["x"].get<float>();
                    }
                    if (Pivot.contains("y") && Pivot["y"].is_number())
                    {
                        Frame.PivotY = Pivot["y"].get<float>();
                    }
                }

                Result->Frames.push_back(Frame);
            }
        }

        if (Result->Info.FrameCount == 0)
        {
            Result->Info.FrameCount = static_cast<uint32>(Result->Frames.size());
        }

        if (Result->Info.FrameWidth == 0 && !Result->Frames.empty())
        {
            Result->Info.FrameWidth = Result->Frames[0].Width;
        }

        if (Result->Info.FrameHeight == 0 && !Result->Frames.empty())
        {
            Result->Info.FrameHeight = Result->Frames[0].Height;
        }

        if (Result->Info.Columns == 0 || Result->Info.Rows == 0)
        {
            if (Root.contains("meta") && Root["meta"].is_object() &&
                Root["meta"].contains("size") && Root["meta"]["size"].is_object())
            {
                const auto& Size = Root["meta"]["size"];
                const uint32 AtlasW =
                    (Size.contains("w") && Size["w"].is_number_unsigned()) ? Size["w"].get<uint32>() : 0;
                const uint32 AtlasH =
                    (Size.contains("h") && Size["h"].is_number_unsigned()) ? Size["h"].get<uint32>() : 0;

                if (Result->Info.FrameWidth > 0 && AtlasW > 0 && Result->Info.Columns == 0)
                {
                    Result->Info.Columns = AtlasW / Result->Info.FrameWidth;
                }

                if (Result->Info.FrameHeight > 0 && AtlasH > 0 && Result->Info.Rows == 0)
                {
                    Result->Info.Rows = AtlasH / Result->Info.FrameHeight;
                }
            }
        }

        return (!Result->AtlasImagePath.empty() && !Result->Frames.empty()) ? Result : nullptr;
    }

    std::shared_ptr<FSubUVAtlasCookedData>
    FSubUVAtlasBuilder::CookAtlas(const FSourceRecord&               Source,
                                  const FIntermediateSubUVAtlasData& Intermediate,
                                  const FTextureBuildSettings&       AtlasTextureSettings)
    {
        FTextureBuilder                     TextureBuilder(Cache);
        std::shared_ptr<FTextureCookedData> AtlasTexture =
            TextureBuilder.Build(Intermediate.AtlasImagePath, AtlasTextureSettings);
        if (!AtlasTexture)
        {
            return nullptr;
        }

        auto Result = std::make_shared<FSubUVAtlasCookedData>();
        Result->SourcePath = Source.NormalizedPath;
        Result->AtlasTexture = AtlasTexture;
        Result->Info = Intermediate.Info;
        Result->Frames = Intermediate.Frames;
        Result->Sequences = Intermediate.Sequences;
        return Result->IsValid() ? Result : nullptr;
    }

    bool FSubUVAtlasBuilder::ReadAllText(const std::filesystem::path& Path, FString& OutText)
    {
        std::ifstream File(Path, std::ios::in | std::ios::binary);
        if (!File)
        {
            return false;
        }

        std::ostringstream Buffer;
        Buffer << File.rdbuf();
        OutText = Buffer.str();
        return true;
    }

    FString FSubUVAtlasBuilder::Trim(const FString& Value)
    {
        const size_t Begin = Value.find_first_not_of(" \t\r\n");
        if (Begin == FString::npos)
        {
            return {};
        }

        const size_t End = Value.find_last_not_of(" \t\r\n");
        return Value.substr(Begin, End - Begin + 1);
    }

    FWString FSubUVAtlasBuilder::ResolveRelativePath(const std::filesystem::path& BasePath,
                                                     const FString&               RelativePath)
    {
        const std::filesystem::path BaseDirectory = BasePath.parent_path();
        return (BaseDirectory / FPaths::PathFromUtf8(RelativePath)).lexically_normal();
    }

} // namespace Asset
