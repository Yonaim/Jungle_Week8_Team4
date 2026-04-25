#pragma once

#include <memory>
#include <utility>

#include "Engine/Asset/Asset.h"
#include "Asset/Cooked/FontAtlasCookedData.h"
#include "Asset/Runtime/FontAtlasRenderResource.h"
#include "RHI/DynamicRHI.h"

using namespace Asset;

class ENGINE_API UFontAtlas : public UAsset
{
    DECLARE_RTTI(UFontAtlas, UAsset)

  public:
    const std::shared_ptr<FFontAtlasCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FFontAtlasCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FFontAtlasRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FFontAtlasRenderResource> InRenderResource)
    {
        RenderResource = std::move(InRenderResource);
    }

    void ResetRenderResource()
    {
        if (RenderResource)
        {
            RenderResource->Reset();
        }
        RenderResource.reset();
    }

    bool LoadFromCooked(const FString& InAssetPath,
                        std::shared_ptr<FFontAtlasCookedData> InCookedData,
                        RHI::FDynamicRHI& InDynamicRHI);

    bool IsValidLowLevel() const override;

  private:
    std::shared_ptr<FFontAtlasCookedData>     CookedData;
    std::shared_ptr<FFontAtlasRenderResource> RenderResource;
};
