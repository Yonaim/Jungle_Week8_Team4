#include <filesystem>
#include <fstream>
#include "Asset/Serialization/CookedDataBinaryIO.h"
#include "Asset/Core/AssetNaming.h"
#include "Core/Misc/Paths.h"

namespace Asset::Binary
{
    namespace
    {
        constexpr uint32 GBinaryMagic = 0x4E494241; // 'ABIN'
        constexpr uint32 GBinaryVersion = 1;

        struct FBinaryHeader
        {
            uint32 Magic = GBinaryMagic;
            uint32 Version = GBinaryVersion;
            uint32 AssetType = 0;
        };

        bool WriteHeader(FArchive& Ar, ECookedBinaryAssetType AssetType)
        {
            FBinaryHeader Header;
            Header.AssetType = static_cast<uint32>(AssetType);
            Ar << Header.Magic;
            Ar << Header.Version;
            Ar << Header.AssetType;
            return Ar.IsOk();
        }

        bool ReadHeader(FArchive& Ar, ECookedBinaryAssetType ExpectedAssetType)
        {
            FBinaryHeader Header;
            Ar << Header.Magic;
            Ar << Header.Version;
            Ar << Header.AssetType;

            return Ar.IsOk() && Header.Magic == GBinaryMagic && Header.Version == GBinaryVersion &&
                   Header.AssetType == static_cast<uint32>(ExpectedAssetType);
        }

        template <typename T>
        bool SaveImpl(const T& Data, const std::filesystem::path& Path,
                      ECookedBinaryAssetType AssetType)
        {
            if (Path.empty())
            {
                return false;
            }

            std::error_code             Ec;
            const std::filesystem::path ParentPath = Path.parent_path();
            if (!ParentPath.empty())
            {
                std::filesystem::create_directories(ParentPath, Ec);
                if (Ec)
                {
                    return false;
                }
            }

            FWindowsBinWriter Writer(Path);
            if (!Writer.IsOk() || !WriteHeader(Writer, AssetType))
            {
                return false;
            }

            T Copy = Data;
            Writer << Copy;
            return Writer.IsOk();
        }
        
        template <typename T>
        bool LoadImpl(const std::filesystem::path& Path, T& OutData,
                      ECookedBinaryAssetType AssetType)
        {
            FWindowsBinReader Reader{Path};
            if (!Reader.IsOk() || !ReadHeader(Reader, AssetType))
            {
                return false;
            }

            T Temp;
            Temp.Reset();

            Reader << Temp;
            if (!Reader.IsOk() || !Temp.IsValid())
            {
                return false;
            }

            OutData = std::move(Temp);
            return true;
        }
    } // namespace

    FString GetSourceExtension(ECookedBinaryAssetType AssetType)
    {
        switch (AssetType)
        {
        case ECookedBinaryAssetType::Texture:
            return Asset::GetPrimarySourceExtension(Asset::EAssetFileKind::Texture);
        case ECookedBinaryAssetType::MaterialLibrary:
            return Asset::GetPrimarySourceExtension(Asset::EAssetFileKind::MaterialLibrary);
        case ECookedBinaryAssetType::StaticMesh:
            return Asset::GetPrimarySourceExtension(Asset::EAssetFileKind::StaticMesh);
        case ECookedBinaryAssetType::FontAtlas:
            return Asset::GetPrimarySourceExtension(Asset::EAssetFileKind::Font);
        case ECookedBinaryAssetType::SubUVAtlas:
            return Asset::GetPrimarySourceExtension(Asset::EAssetFileKind::TextureAtlas);
        default:
            return {};
        }
    }

    FString GetBinaryExtension(ECookedBinaryAssetType AssetType)
    {
        switch (AssetType)
        {
        case ECookedBinaryAssetType::Texture:
            return Asset::GetPrimaryBakedExtension(Asset::EAssetFileKind::Texture);
        case ECookedBinaryAssetType::MaterialLibrary:
            return Asset::GetPrimaryBakedExtension(Asset::EAssetFileKind::MaterialLibrary);
        case ECookedBinaryAssetType::StaticMesh:
            return Asset::GetPrimaryBakedExtension(Asset::EAssetFileKind::StaticMesh);
        case ECookedBinaryAssetType::FontAtlas:
            return Asset::GetPrimaryBakedExtension(Asset::EAssetFileKind::Font);
        case ECookedBinaryAssetType::SubUVAtlas:
            return Asset::GetPrimaryBakedExtension(Asset::EAssetFileKind::TextureAtlas);
        default:
            return {};
        }
    }

    bool TryGetAssetTypeFromSourcePath(const FString& SourcePath, ECookedBinaryAssetType& OutType)
    {
        switch (Asset::ClassifyAssetPath(SourcePath))
        {
        case Asset::EAssetFileKind::Texture:
            OutType = ECookedBinaryAssetType::Texture;
            return true;
        case Asset::EAssetFileKind::MaterialLibrary:
            OutType = ECookedBinaryAssetType::MaterialLibrary;
            return true;
        case Asset::EAssetFileKind::StaticMesh:
            OutType = ECookedBinaryAssetType::StaticMesh;
            return true;
        case Asset::EAssetFileKind::Font:
            OutType = ECookedBinaryAssetType::FontAtlas;
            return true;
        case Asset::EAssetFileKind::TextureAtlas:
            OutType = ECookedBinaryAssetType::SubUVAtlas;
            return true;
        default:
            OutType = ECookedBinaryAssetType::Unknown;
            return false;
        }
    }

    FString MakeBinaryPathFromSourcePath(const FString& SourcePath)
    {
        return Asset::MakeBakedAssetPath(SourcePath);
    }

    bool SaveTexture(const FTextureCookedData& Data, const FString& Path)
    {
        return SaveTexture(Data, FPaths::PathFromUtf8(Path));
    }

    bool SaveTexture(const FTextureCookedData& Data, const std::filesystem::path& Path)
    {
        return SaveImpl(Data, Path, ECookedBinaryAssetType::Texture);
    }

    bool LoadTexture(const FString& Path, FTextureCookedData& OutData)
    {
        return LoadTexture(FPaths::PathFromUtf8(Path), OutData);
    }

    bool LoadTexture(const std::filesystem::path& Path, FTextureCookedData& OutData)
    {
        return LoadImpl(Path, OutData, ECookedBinaryAssetType::Texture);
    }

    bool SaveMaterialLibrary(const FMtlCookedLibraryData& Data, const FString& Path)
    {
        return SaveMaterialLibrary(Data, FPaths::PathFromUtf8(Path));
    }

    bool SaveMaterialLibrary(const FMtlCookedLibraryData&       Data,
                             const std::filesystem::path& Path)
    {
        return SaveImpl(Data, Path, ECookedBinaryAssetType::MaterialLibrary);
    }

    bool LoadMaterialLibrary(const FString& Path, FMtlCookedLibraryData& OutData)
    {
        return LoadMaterialLibrary(FPaths::PathFromUtf8(Path), OutData);
    }

    bool LoadMaterialLibrary(const std::filesystem::path& Path, FMtlCookedLibraryData& OutData)
    {
        return LoadImpl(Path, OutData, ECookedBinaryAssetType::MaterialLibrary);
    }

    bool SaveStaticMesh(const FObjCookedData& Data, const FString& Path)
    {
        return SaveStaticMesh(Data, FPaths::PathFromUtf8(Path));
    }

    bool SaveStaticMesh(const FObjCookedData& Data, const std::filesystem::path& Path)
    {
        return SaveImpl(Data, Path, ECookedBinaryAssetType::StaticMesh);
    }

    bool LoadStaticMesh(const FString& Path, FObjCookedData& OutData)
    {
        return LoadStaticMesh(FPaths::PathFromUtf8(Path), OutData);
    }

    bool LoadStaticMesh(const std::filesystem::path& Path, FObjCookedData& OutData)
    {
        return LoadImpl(Path, OutData, ECookedBinaryAssetType::StaticMesh);
    }

    bool SaveFontAtlas(const FFontAtlasCookedData& Data, const FString& Path)
    {
        return SaveFontAtlas(Data, FPaths::PathFromUtf8(Path));
    }

    bool SaveFontAtlas(const FFontAtlasCookedData& Data, const std::filesystem::path& Path)
    {
        return SaveImpl(Data, Path, ECookedBinaryAssetType::FontAtlas);
    }

    bool LoadFontAtlas(const FString& Path, FFontAtlasCookedData& OutData)
    {
        return LoadFontAtlas(FPaths::PathFromUtf8(Path), OutData);
    }

    bool LoadFontAtlas(const std::filesystem::path& Path, FFontAtlasCookedData& OutData)
    {
        return LoadImpl(Path, OutData, ECookedBinaryAssetType::FontAtlas);
    }

    bool SaveSubUVAtlas(const FSubUVAtlasCookedData& Data, const FString& Path)
    {
        return SaveSubUVAtlas(Data, FPaths::PathFromUtf8(Path));
    }

    bool SaveSubUVAtlas(const FSubUVAtlasCookedData& Data, const std::filesystem::path& Path)
    {
        return SaveImpl(Data, Path, ECookedBinaryAssetType::SubUVAtlas);
    }

    bool LoadSubUVAtlas(const FString& Path, FSubUVAtlasCookedData& OutData)
    {
        return LoadSubUVAtlas(FPaths::PathFromUtf8(Path), OutData);
    }

    bool LoadSubUVAtlas(const std::filesystem::path& Path, FSubUVAtlasCookedData& OutData)
    {
        return LoadImpl(Path, OutData, ECookedBinaryAssetType::SubUVAtlas);
    }

} // namespace Asset::Binary
