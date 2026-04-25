// 텍스처 영역의 세부 동작을 구현합니다.
#include "Texture/Texture2D.h"
#include "Object/ObjectFactory.h"
#include "Editor/UI/EditorConsolePanel.h"
#include "Platform/Paths.h"
#include "Profiling/MemoryStats.h"

#include <d3d11.h>

IMPLEMENT_CLASS(UTexture2D, UAsset)

namespace
{
FString GetAssetStem(const FString& AssetPath)
{
    return FPaths::ToUtf8(std::filesystem::path(FPaths::ToWide(AssetPath)).stem().wstring());
}
}

UTexture2D::~UTexture2D()
{
    ReleaseGPU();
}

bool UTexture2D::LoadFromCooked(const FString& AssetPath, const Asset::FTextureCookedData& CookedData, ID3D11Device* Device)
{
    if (!Device || !CookedData.IsValid())
    {
        return false;
    }

    ResetAsset();

    DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
    switch (CookedData.Format)
    {
    case Asset::EPixelFormat::RGBA8:
        Format = CookedData.bSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    default:
        UE_LOG("[ERROR] Unsupported cooked texture format for asset: %s", AssetPath.c_str());
        return false;
    }

    D3D11_TEXTURE2D_DESC Desc = {};
    Desc.Width = CookedData.Width;
    Desc.Height = CookedData.Height;
    Desc.MipLevels = 1;
    Desc.ArraySize = 1;
    Desc.Format = Format;
    Desc.SampleDesc.Count = 1;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA InitialData = {};
    InitialData.pSysMem = CookedData.Pixels.data();
    InitialData.SysMemPitch = CookedData.Width * 4u;

    ID3D11Texture2D* TextureResource = nullptr;
    HRESULT Hr = Device->CreateTexture2D(&Desc, &InitialData, &TextureResource);
    if (FAILED(Hr) || !TextureResource)
    {
        UE_LOG("[ERROR] Failed to create texture resource: %s", AssetPath.c_str());
        return false;
    }

    Hr = Device->CreateShaderResourceView(TextureResource, nullptr, &SRV);
    if (FAILED(Hr))
    {
        TextureResource->Release();
        UE_LOG("[ERROR] Failed to create texture SRV: %s", AssetPath.c_str());
        return false;
    }

    TrackedTextureMemory = MemoryStats::CalculateTextureMemory(TextureResource);
    if (TrackedTextureMemory > 0)
    {
        MemoryStats::AddTextureMemory(TrackedTextureMemory);
    }
    TextureResource->Release();

    Width = CookedData.Width;
    Height = CookedData.Height;
    SetAssetPathFileName(AssetPath);
    SetAssetName(GetAssetStem(AssetPath));
    SetLoaded(true);
    return true;
}

void UTexture2D::ReleaseGPU()
{
    if (TrackedTextureMemory > 0)
    {
        MemoryStats::SubTextureMemory(TrackedTextureMemory);
        TrackedTextureMemory = 0;
    }

    if (SRV)
    {
        SRV->Release();
        SRV = nullptr;
    }
}

void UTexture2D::ResetAsset()
{
    ReleaseGPU();
    Width = 0;
    Height = 0;
    UAsset::ResetAsset();
}
