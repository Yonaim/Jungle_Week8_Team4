#pragma once

#include <filesystem>
#include <sstream>

#include "Asset/Cache/AssetKey.h"
#include "Asset/Cache/BuildSettings.h"
#include "Asset/Intermediate/IntermediateFontAtlasData.h"
#include "Asset/Intermediate/IntermediateMtlData.h"
#include "Asset/Intermediate/IntermediateObjData.h"
#include "Asset/Intermediate/IntermediateSubUVAtlasData.h"
#include "Asset/Intermediate/IntermediateTextureData.h"
#include "Core/Misc/Paths.h"

namespace Asset
{
    namespace KeyUtils
    {
        inline FString FinalizeHash(size_t Seed)
        {
            std::ostringstream Oss;
            Oss << std::hex << Seed;
            return Oss.str();
        }
        inline FString PathToKeyString(const std::filesystem::path& Path)
        {
            return FPaths::Utf8FromPath(Path);
        }

        inline void HashVector(size_t& Seed, const FVector& V)
        {
            KeyHash::Combine(Seed, V.X);
            KeyHash::Combine(Seed, V.Y);
            KeyHash::Combine(Seed, V.Z);
        }
        inline void HashVector2(size_t& Seed, const FVector2& V)
        {
            KeyHash::Combine(Seed, V.X);
            KeyHash::Combine(Seed, V.Y);
        }

        inline FTextureIntermediateKey MakeIntermediateKey(const FIntermediateTextureData& Data)
        {
            size_t Seed = 0;
            KeyHash::Combine(Seed, Data.Width);
            KeyHash::Combine(Seed, Data.Height);
            KeyHash::Combine(Seed, Data.Channels);
            KeyHash::Combine(Seed, static_cast<uint32>(Data.Format));
            KeyHash::Combine(Seed, Data.Pixels.size());
            for (uint8 Byte : Data.Pixels)
                KeyHash::Combine(Seed, Byte);
            return {FinalizeHash(Seed)};
        }

        inline void HashMaterialData(size_t& Seed, const FIntermediateMtlData& Data)
        {
            KeyHash::CombineString(Seed, Data.Name);
            HashVector(Seed, Data.DiffuseColor);
            HashVector(Seed, Data.AmbientColor);
            HashVector(Seed, Data.SpecularColor);
            KeyHash::Combine(Seed, Data.Shininess);
            KeyHash::Combine(Seed, Data.Opacity);
            for (const auto& Ref : Data.TextureRefs)
            {
                KeyHash::CombineString(Seed, Ref.SlotName);
                KeyHash::CombineString(Seed, PathToKeyString(Ref.TexturePath));
            }
        }

        inline FMaterialIntermediateKey MakeIntermediateKey(const FIntermediateMtlData& Data)
        {
            size_t Seed = 0;
            HashMaterialData(Seed, Data);
            return {FinalizeHash(Seed)};
        }

        inline FMaterialIntermediateKey MakeIntermediateKey(const FIntermediateMtlLibraryData& Data)
        {
            size_t Seed = 0;
            KeyHash::CombineString(Seed, PathToKeyString(Data.SourcePath));
            for (const auto& Material : Data.Materials)
            {
                HashMaterialData(Seed, Material);
            }
            return {FinalizeHash(Seed)};
        }

        inline FStaticMeshIntermediateKey MakeIntermediateKey(const FIntermediateObjData& Data)
        {
            size_t Seed = 0;
            for (const auto& V : Data.Positions)
                HashVector(Seed, V);
            for (const auto& V : Data.Colors)
                HashVector(Seed, V);
            for (const auto& V : Data.Normals)
                HashVector(Seed, V);
            for (const auto& UV : Data.UVs)
                HashVector2(Seed, UV);
            for (const auto& LibraryPath : Data.MaterialLibraries)
            {
                KeyHash::CombineString(Seed, PathToKeyString(LibraryPath));
            }

            for (const auto& Face : Data.Faces)
            {
                KeyHash::CombineString(Seed, Face.MaterialName);
                KeyHash::Combine(Seed, Face.MaterialLibraryIndex);
                for (const auto& Vertex : Face.Vertices)
                {
                    KeyHash::Combine(Seed, Vertex.PositionIndex);
                    KeyHash::Combine(Seed, Vertex.NormalIndex);
                    KeyHash::Combine(Seed, Vertex.UVIndex);
                }
            }
            return {FinalizeHash(Seed)};
        }

        inline FSubUVAtlasIntermediateKey
        MakeIntermediateKey(const FIntermediateSubUVAtlasData& Data)
        {
            size_t Seed = 0;
            KeyHash::CombineString(Seed, Data.Info.Name);
            KeyHash::Combine(Seed, Data.Info.FrameWidth);
            KeyHash::Combine(Seed, Data.Info.FrameHeight);
            KeyHash::Combine(Seed, Data.Info.Columns);
            KeyHash::Combine(Seed, Data.Info.Rows);
            KeyHash::Combine(Seed, Data.Info.FrameCount);
            KeyHash::Combine(Seed, Data.Info.FPS);
            KeyHash::Combine(Seed, Data.Info.bLoop);
            KeyHash::CombineWString(Seed, Data.AtlasImagePath);
            for (const auto& Frame : Data.Frames)
            {
                KeyHash::Combine(Seed, Frame.Id);
                KeyHash::Combine(Seed, Frame.X);
                KeyHash::Combine(Seed, Frame.Y);
                KeyHash::Combine(Seed, Frame.Width);
                KeyHash::Combine(Seed, Frame.Height);
                KeyHash::Combine(Seed, Frame.PivotX);
                KeyHash::Combine(Seed, Frame.PivotY);
                KeyHash::Combine(Seed, Frame.Duration);
            }
            for (const auto& Pair : Data.Sequences)
            {
                KeyHash::CombineString(Seed, Pair.first);
                KeyHash::Combine(Seed, Pair.second.StartFrame);
                KeyHash::Combine(Seed, Pair.second.EndFrame);
                KeyHash::Combine(Seed, Pair.second.bLoop);
            }
            return {FinalizeHash(Seed)};
        }

        inline FFontAtlasIntermediateKey MakeIntermediateKey(const FIntermediateFontAtlasData& Data)
        {
            size_t Seed = 0;
            KeyHash::CombineString(Seed, Data.Info.Face);
            KeyHash::Combine(Seed, Data.Info.Size);
            KeyHash::Combine(Seed, Data.Info.bBold);
            KeyHash::Combine(Seed, Data.Info.bItalic);
            KeyHash::Combine(Seed, Data.Info.bUnicode);
            KeyHash::Combine(Seed, Data.Common.LineHeight);
            KeyHash::Combine(Seed, Data.Common.Base);
            KeyHash::Combine(Seed, Data.Common.ScaleW);
            KeyHash::Combine(Seed, Data.Common.ScaleH);
            KeyHash::Combine(Seed, Data.Common.Pages);
            KeyHash::Combine(Seed, Data.Common.bPacked);
            KeyHash::CombineWString(Seed, Data.AtlasImagePath);
            for (const auto& Pair : Data.Glyphs)
            {
                KeyHash::Combine(Seed, Pair.first);
                const auto& G = Pair.second;
                KeyHash::Combine(Seed, G.Id);
                KeyHash::Combine(Seed, G.X);
                KeyHash::Combine(Seed, G.Y);
                KeyHash::Combine(Seed, G.Width);
                KeyHash::Combine(Seed, G.Height);
                KeyHash::Combine(Seed, G.XOffset);
                KeyHash::Combine(Seed, G.YOffset);
                KeyHash::Combine(Seed, G.XAdvance);
                KeyHash::Combine(Seed, G.Page);
                KeyHash::Combine(Seed, G.Channel);
            }
            return {FinalizeHash(Seed)};
        }

        inline FTextureCookedKey MakeCookedKey(const FTextureIntermediateKey& IntermediateKey,
                                               const FTextureBuildSettings&   Settings,
                                               uint32                         CookVersion = 1)
        {
            return {IntermediateKey, CookVersion, Settings.ToKeyString()};
        }

        inline FMaterialCookedKey MakeCookedKey(const FMaterialIntermediateKey& IntermediateKey,
                                                uint32                          CookVersion = 1,
                                                const FString& BuildKey = "MaterialLibraryCook")
        {
            return {IntermediateKey, CookVersion, BuildKey};
        }

        inline FStaticMeshCookedKey MakeCookedKey(const FStaticMeshIntermediateKey& IntermediateKey,
                                                  const FStaticMeshBuildSettings&   Settings,
                                                  uint32                            CookVersion = 1)
        {
            return {IntermediateKey, CookVersion, Settings.ToKeyString()};
        }

        inline FSubUVAtlasCookedKey MakeCookedKey(const FSubUVAtlasIntermediateKey& IntermediateKey,
                                                  const FTextureBuildSettings&      Settings,
                                                  uint32                            CookVersion = 1)
        {
            return {IntermediateKey, CookVersion, Settings.ToKeyString()};
        }

        inline FFontAtlasCookedKey MakeCookedKey(const FFontAtlasIntermediateKey& IntermediateKey,
                                                 const FTextureBuildSettings&     Settings,
                                                 uint32                           CookVersion = 1)
        {
            return {IntermediateKey, CookVersion, Settings.ToKeyString()};
        }
    } // namespace KeyUtils
} // namespace Asset
