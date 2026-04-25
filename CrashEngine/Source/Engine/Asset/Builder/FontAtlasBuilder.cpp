#include "Asset/Builder/FontAtlasBuilder.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include "ThirdParty/nlohmann/json.hpp"

#include "Asset/Cache/AssetKeyUtils.h"
#include "Asset/Cache/AssetBuildCache.h"
#include "Asset/Core/AssetNaming.h"
#include "Asset/Serialization/CookedDataBinaryIO.h"
#include "Core/Misc/Paths.h"

namespace Asset
{
    std::shared_ptr<FFontAtlasCookedData>
    FFontAtlasBuilder::Build(const std::filesystem::path& Path,
                             const FTextureBuildSettings& AtlasTextureSettings)
    {
        LastBuildReport.Reset();
        const FSourceRecord* Source = Cache.GetSource(FFontAtlasAssetTag{}, Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        const FString BakedFontPath = MakeBakedAssetPath(FPaths::Utf8FromPath(Source->NormalizedPath));
        if (!BakedFontPath.empty())
        {
            FFontAtlasCookedData BakedData;
            if (Binary::LoadFontAtlas(BakedFontPath, BakedData) && BakedData.IsValid())
            {
                LastBuildReport.bUsedCachedCooked = true;
                LastBuildReport.ResultSource = EAssetBuildResultSource::CookedCache;
                return std::make_shared<FFontAtlasCookedData>(std::move(BakedData));
            }
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FFontAtlasAssetTag{});
        std::shared_ptr<FIntermediateFontAtlasData> Intermediate = ParseFontAtlas(*Source);
        if (!Intermediate)
        {
            return nullptr;
        }

        const FFontAtlasIntermediateKey IntermediateKey =
            KeyUtils::MakeIntermediateKey(*Intermediate);
        std::shared_ptr<FIntermediateFontAtlasData> CachedIntermediate =
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

        const FFontAtlasCookedKey CookedKey =
            KeyUtils::MakeCookedKey(IntermediateKey, AtlasTextureSettings);

        auto& CookedCache = Cache.GetCookedCache(FFontAtlasAssetTag{});
        std::shared_ptr<FFontAtlasCookedData> Cooked = CookedCache.Find(CookedKey);
        if (!Cooked)
        {
            LastBuildReport.bBuiltNewCooked = true;
            Cooked = CookFontAtlas(*Source, *Intermediate, AtlasTextureSettings);
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

        if (Cooked && !BakedFontPath.empty())
        {
            Binary::SaveFontAtlas(*Cooked, BakedFontPath);
        }

        return Cooked;
    }

    std::shared_ptr<FIntermediateFontAtlasData>
    FFontAtlasBuilder::ParseFontAtlas(const FSourceRecord& Source)
    {
        FString Text;
        if (!ReadAllText(Source.NormalizedPath, Text))
        {
            return nullptr;
        }

        nlohmann::json J = nlohmann::json::parse(Text, nullptr, false);
        if (J.is_discarded() || !J.is_object())
        {
            return nullptr;
        }

        auto Result = std::make_shared<FIntermediateFontAtlasData>();

        // info
        if (J.contains("info") && J["info"].is_object())
        {
            const auto& Info = J["info"];
            Result->Info.Face = Info.value("face", "");
            Result->Info.Size = Info.value("size", 0);
            Result->Info.bBold = Info.value("bold", 0) != 0;
            Result->Info.bItalic = Info.value("italic", 0) != 0;
            Result->Info.bUnicode = Info.value("unicode", 0) != 0;
        }

        // common
        if (J.contains("common") && J["common"].is_object())
        {
            const auto& Common = J["common"];
            Result->Common.LineHeight = Common.value("lineHeight", 0u);
            Result->Common.Base = Common.value("base", 0u);
            Result->Common.ScaleW = Common.value("scaleW", 0u);
            Result->Common.ScaleH = Common.value("scaleH", 0u);
            Result->Common.Pages = Common.value("pages", 0u);
            Result->Common.bPacked = Common.value("packed", 0) != 0;
        }

        // atlas page path
        if (J.contains("pages") && J["pages"].is_array() && !J["pages"].empty())
        {
            const FString RelativePagePath = J["pages"][0].get<std::string>();
            Result->AtlasImagePath = ResolveRelativePath(Source.NormalizedPath, RelativePagePath);
        }

        // chars
        if (J.contains("chars") && J["chars"].is_array())
        {
            for (const auto& CharNode : J["chars"])
            {
                if (!CharNode.is_object())
                {
                    continue;
                }

                FFontGlyph Glyph;
                Glyph.Id = CharNode.value("id", 0u);
                Glyph.X = CharNode.value("x", 0u);
                Glyph.Y = CharNode.value("y", 0u);
                Glyph.Width = CharNode.value("width", 0u);
                Glyph.Height = CharNode.value("height", 0u);
                Glyph.XOffset = CharNode.value("xoffset", 0);
                Glyph.YOffset = CharNode.value("yoffset", 0);
                Glyph.XAdvance = CharNode.value("xadvance", 0);
                Glyph.Page = CharNode.value("page", 0u);
                Glyph.Channel = CharNode.value("chnl", 0u);

                Result->Glyphs[Glyph.Id] = Glyph;
            }
        }

        if (Result->AtlasImagePath.empty() || Result->Glyphs.empty())
        {
            return nullptr;
        }

        return Result;
    }

    std::shared_ptr<FFontAtlasCookedData>
    FFontAtlasBuilder::CookFontAtlas(const FSourceRecord&              Source,
                                     const FIntermediateFontAtlasData& Intermediate,
                                     const FTextureBuildSettings&      AtlasTextureSettings)
    {
        FTextureBuilder                     TextureBuilder(Cache);
        std::shared_ptr<FTextureCookedData> AtlasTexture =
            TextureBuilder.Build(Intermediate.AtlasImagePath, AtlasTextureSettings);
        if (!AtlasTexture)
        {
            return nullptr;
        }

        auto Result = std::make_shared<FFontAtlasCookedData>();
        Result->SourcePath = Source.NormalizedPath;
        Result->AtlasTexture = AtlasTexture;
        Result->Info = Intermediate.Info;
        Result->Common = Intermediate.Common;
        Result->Glyphs = Intermediate.Glyphs;
        return Result->IsValid() ? Result : nullptr;
    }

    bool FFontAtlasBuilder::ReadAllText(const std::filesystem::path& Path, FString& OutText)
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

    FString FFontAtlasBuilder::Trim(const FString& Value)
    {
        const size_t Begin = Value.find_first_not_of(" \t\r\n");
        if (Begin == FString::npos)
        {
            return {};
        }

        const size_t End = Value.find_last_not_of(" \t\r\n");
        return Value.substr(Begin, End - Begin + 1);
    }

    FString FFontAtlasBuilder::ExtractQuotedValue(const FString& Line)
    {
        const size_t Begin = Line.find('"');
        if (Begin == FString::npos)
        {
            return {};
        }
        const size_t End = Line.find('"', Begin + 1);
        if (End == FString::npos || End <= Begin)
        {
            return {};
        }
        return Line.substr(Begin + 1, End - Begin - 1);
    }

    bool FFontAtlasBuilder::TryParseKeyValueLine(const FString&          Line,
                                                 TMap<FString, FString>& OutPairs)
    {
        std::istringstream Iss(Line);
        FString            Token;
        bool               bParsedAny = false;
        while (Iss >> Token)
        {
            const size_t EqualsPos = Token.find('=');
            if (EqualsPos == FString::npos)
            {
                continue;
            }

            FString Key = Token.substr(0, EqualsPos);
            FString Value = Token.substr(EqualsPos + 1);

            if (!Value.empty() && Value.front() == '"' && Value.back() != '"')
            {
                FString Tail;
                while (Iss >> Tail)
                {
                    Value += " " + Tail;
                    if (!Tail.empty() && Tail.back() == '"')
                    {
                        break;
                    }
                }
            }

            if (Value.size() >= 2 && Value.front() == '"' && Value.back() == '"')
            {
                Value = Value.substr(1, Value.size() - 2);
            }

            OutPairs[Key] = Value;
            bParsedAny = true;
        }
        return bParsedAny;
    }

    FWString FFontAtlasBuilder::ResolveRelativePath(const std::filesystem::path& BasePath,
                                                    const FString&               RelativePath)
    {
        const std::filesystem::path BaseDirectory = BasePath.parent_path();
        return (BaseDirectory / FPaths::PathFromUtf8(RelativePath)).lexically_normal();
    }

} // namespace Asset
