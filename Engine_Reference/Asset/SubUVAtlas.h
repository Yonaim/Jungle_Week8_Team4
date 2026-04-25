#pragma once

#include <memory>
#include <utility>

#include "Engine/Asset/Asset.h"
#include "Asset/Cooked/SubUVAtlasCookedData.h"
#include "Asset/Runtime/SubUVAtlasRenderResource.h"
#include "RHI/DynamicRHI.h"

using namespace Asset;

class ENGINE_API USubUVAtlas : public UAsset
{
    DECLARE_RTTI(USubUVAtlas, UAsset)

  public:
    const std::shared_ptr<FSubUVAtlasCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FSubUVAtlasCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FSubUVAtlasRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FSubUVAtlasRenderResource> InRenderResource)
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

    bool LoadFromCooked(const FString&                         InAssetPath,
                        std::shared_ptr<FSubUVAtlasCookedData> InCookedData,
                        RHI::FDynamicRHI&                      InDynamicRHI);

    bool IsValidLowLevel() const override;

  private:
    std::shared_ptr<FSubUVAtlasCookedData>     CookedData;
    std::shared_ptr<FSubUVAtlasRenderResource> RenderResource;
};
