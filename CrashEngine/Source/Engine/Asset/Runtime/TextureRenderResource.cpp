#include "Asset/Runtime/TextureRenderResource.h"

namespace Asset
{
    std::shared_ptr<FTextureRenderResource>
    FTextureRenderResource::Create(const FTextureCookedData& CookedData, RHI::FDynamicRHI& RHI)
    {
        if (!CookedData.IsValid())
        {
            return nullptr;
        }

        RHI::FTextureDesc Desc;
        Desc.Width = CookedData.Width;
        Desc.Height = CookedData.Height;
        Desc.MipLevels = 1;
        Desc.ArraySize = 1;
        Desc.Format = ResolvePixelFormat(CookedData.Channels);
        Desc.BindFlags = RHI::ETextureBindFlags::ShaderResource;
        Desc.Usage = RHI::EBufferUsage::Default;
        Desc.CPUAccessFlags = RHI::ECPUAccessFlags::None;

        if (Desc.Format == RHI::EPixelFormat::Unknown)
        {
            return nullptr;
        }

        const uint32                        Pitch = CookedData.Width * CookedData.Channels;
        std::shared_ptr<RHI::FRHITexture2D> Texture2D =
            RHI.CreateTexture2D(Desc, CookedData.Pixels.data(), Pitch);
        if (Texture2D == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<FTextureRenderResource> Resource =
            std::make_shared<FTextureRenderResource>();
        Resource->Width = CookedData.Width;
        Resource->Height = CookedData.Height;
        Resource->Format = Desc.Format;
        Resource->Texture = std::move(Texture2D);
        return Resource;
    }

    bool FTextureRenderResource::IsValid() const { return Texture != nullptr; }

    void FTextureRenderResource::Reset()
    {
        Width = 0;
        Height = 0;
        Format = RHI::EPixelFormat::Unknown;
        Texture.reset();
    }

    RHI::EPixelFormat FTextureRenderResource::ResolvePixelFormat(uint32 Channels)
    {
        switch (Channels)
        {
        case 1:
            return RHI::EPixelFormat::R8;
        case 2:
            return RHI::EPixelFormat::RG8;
        case 3:
            return RHI::EPixelFormat::RGB8;
        case 4:
            return RHI::EPixelFormat::RGBA8;
        default:
            return RHI::EPixelFormat::Unknown;
        }
    }
} // namespace Asset
