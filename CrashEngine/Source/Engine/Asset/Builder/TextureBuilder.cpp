#include "Asset/Builder/TextureBuilder.h"

#include <cstring>
#include <filesystem>

#include "Asset/Cache/AssetKeyUtils.h"
#include "Asset/Core/AssetNaming.h"
#include "Asset/Serialization/CookedDataBinaryIO.h"
#include "Asset/Source/SourceLoader.h"
#include "Core/Misc/Paths.h"

#include <wincodec.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace Asset
{
    namespace
    {
        bool DecodeTextureWithWIC(const std::filesystem::path& Path, FIntermediateTextureData& OutData)
        {
            HRESULT InitHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (FAILED(InitHr) && InitHr != RPC_E_CHANGED_MODE)
            {
                return false;
            }

            ComPtr<IWICImagingFactory> Factory;
            HRESULT Hr = CoCreateInstance(
                CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Factory));
            if (FAILED(Hr))
            {
                return false;
            }

            ComPtr<IWICBitmapDecoder> Decoder;
            Hr = Factory->CreateDecoderFromFilename(
                Path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &Decoder);
            if (FAILED(Hr))
            {
                return false;
            }

            ComPtr<IWICBitmapFrameDecode> Frame;
            Hr = Decoder->GetFrame(0, &Frame);
            if (FAILED(Hr))
            {
                return false;
            }

            UINT Width = 0;
            UINT Height = 0;
            Hr = Frame->GetSize(&Width, &Height);
            if (FAILED(Hr) || Width == 0 || Height == 0)
            {
                return false;
            }

            ComPtr<IWICFormatConverter> Converter;
            Hr = Factory->CreateFormatConverter(&Converter);
            if (FAILED(Hr))
            {
                return false;
            }

            Hr = Converter->Initialize(
                Frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0,
                WICBitmapPaletteTypeCustom);
            if (FAILED(Hr))
            {
                return false;
            }

            const UINT Stride = Width * 4u;
            const UINT BufferSize = Stride * Height;
            OutData.Width = Width;
            OutData.Height = Height;
            OutData.Channels = 4;
            OutData.Format = EPixelFormat::RGBA8;
            OutData.Pixels.resize(BufferSize);

            Hr = Converter->CopyPixels(nullptr, Stride, BufferSize, OutData.Pixels.data());
            return SUCCEEDED(Hr);
        }
    } // namespace

    FTextureBuilder::FTextureBuilder(FAssetBuildCache& InCache) : Cache(InCache) {}

    std::shared_ptr<FTextureCookedData>
    FTextureBuilder::Build(const std::filesystem::path& Path, const FTextureBuildSettings& Settings)
    {
        LastBuildReport.Reset();
        const FSourceRecord* Source = Cache.GetSource(FTextureAssetTag{}, Path);
        if (Source == nullptr)
        {
            return nullptr;
        }

        const FString BakedTexturePath = MakeBakedAssetPath(FPaths::Utf8FromPath(Source->NormalizedPath));
        if (!BakedTexturePath.empty())
        {
            FTextureCookedData BakedData;
            if (Binary::LoadTexture(BakedTexturePath, BakedData) && BakedData.IsValid())
            {
                LastBuildReport.bUsedCachedCooked = true;
                LastBuildReport.ResultSource = EAssetBuildResultSource::CookedCache;
                return std::make_shared<FTextureCookedData>(std::move(BakedData));
            }
        }

        auto& IntermediateCache = Cache.GetIntermediateCache(FTextureAssetTag{});

        std::shared_ptr<FIntermediateTextureData> Intermediate;
        FTextureIntermediateKey                   IntermediateKey;

        Intermediate = std::make_shared<FIntermediateTextureData>();
        if (!DecodeTexture(*Source, *Intermediate))
        {
            return nullptr;
        }

        IntermediateKey = KeyUtils::MakeIntermediateKey(*Intermediate);

        std::shared_ptr<FIntermediateTextureData> CachedIntermediate =
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

        const FTextureCookedKey CookedKey = KeyUtils::MakeCookedKey(IntermediateKey, Settings);

        auto&                               CookedCache = Cache.GetCookedCache(FTextureAssetTag{});
        std::shared_ptr<FTextureCookedData> Cooked = CookedCache.Find(CookedKey);

        if (!Cooked)
        {
            LastBuildReport.bBuiltNewCooked = true;
            Cooked = CookTexture(*Source, *Intermediate, Settings);
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

        if (Cooked && !BakedTexturePath.empty())
        {
            Binary::SaveTexture(*Cooked, BakedTexturePath);
        }

        return Cooked;
    }

    bool FTextureBuilder::DecodeTexture(const FSourceRecord&      Source,
                                        FIntermediateTextureData& OutData) const
    {
        return DecodeTextureWithWIC(Source.NormalizedPath, OutData);
    }

    std::shared_ptr<FTextureCookedData>
    FTextureBuilder::CookTexture(const FSourceRecord&            Source,
                                 const FIntermediateTextureData& Intermediate,
                                 const FTextureBuildSettings&    Settings) const
    {
        if (Intermediate.Width == 0 || Intermediate.Height == 0)
        {
            return nullptr;
        }

        if (Intermediate.Pixels.empty())
        {
            return nullptr;
        }

        auto Result = std::make_shared<FTextureCookedData>();
        Result->SourcePath = Source.NormalizedPath;
        Result->Width = Intermediate.Width;
        Result->Height = Intermediate.Height;
        Result->Channels = Intermediate.Channels;
        Result->Format = Intermediate.Format;
        Result->Pixels = Intermediate.Pixels;
        Result->bSRGB = Settings.bSRGB;

        return Result->IsValid() ? Result : nullptr;
    }

} // namespace Asset
