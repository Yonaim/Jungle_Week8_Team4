#include "Asset/Runtime/FontAtlasRenderResource.h"
#include "Asset/Runtime/TextureRenderResource.h"

namespace Asset
{
    std::shared_ptr<FFontAtlasRenderResource>
    FFontAtlasRenderResource::Create(const FFontAtlasCookedData& CookedData, RHI::FDynamicRHI& RHI)
    {
        if (!CookedData.IsValid() || CookedData.AtlasTexture == nullptr)
        {
            return nullptr;
        }

        std::shared_ptr<FTextureRenderResource> TextureResource =
            FTextureRenderResource::Create(*CookedData.AtlasTexture, RHI);
        if (TextureResource == nullptr || !TextureResource->IsValid())
        {
            return nullptr;
        }

        std::shared_ptr<FFontAtlasRenderResource> Resource =
            std::make_shared<FFontAtlasRenderResource>();
        Resource->Info = CookedData.Info;
        Resource->Common = CookedData.Common;
        Resource->Glyphs = CookedData.Glyphs;
        Resource->AtlasTexture = std::move(TextureResource->Texture);
        return Resource;
    }

    bool FFontAtlasRenderResource::IsValid() const { return AtlasTexture != nullptr; }

    void FFontAtlasRenderResource::Reset()
    {
        Info = {};
        Common = {};
        Glyphs.clear();
        AtlasTexture.reset();
    }
} // namespace Asset
