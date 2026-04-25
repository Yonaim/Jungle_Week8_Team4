#pragma once

#include <memory>

#include "Asset/Cooked/TextureCookedData.h"
#include "Renderer/D3D11/D3D11Common.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHITexture.h"
#include "RHI/D3D11/D3D11Texture.h"

namespace Asset
{
    struct FTextureRenderResource
    {
        uint32            Width = 0;
        uint32            Height = 0;
        RHI::EPixelFormat Format = RHI::EPixelFormat::Unknown;

        std::shared_ptr<RHI::FRHITexture> Texture;

        static std::shared_ptr<FTextureRenderResource> Create(const FTextureCookedData& CookedData,
                                                              RHI::FDynamicRHI&         RHI);

        bool IsValid() const;
        void Reset();

        ID3D11ShaderResourceView* GetSRV() const 
        { 
            if (!Texture)
            {
                return nullptr;
            }
            auto* D3D11Tex = dynamic_cast<RHI::D3D11::FD3D11Texture2D*>(Texture.get());
            return D3D11Tex ? D3D11Tex->GetSRV() : nullptr;
        }

      private:
        static RHI::EPixelFormat ResolvePixelFormat(uint32 Channels);
    };
} // namespace Asset
