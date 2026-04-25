#pragma once

#include <memory>
#include <utility>
#include <filesystem>

#include "Engine/Asset/Asset.h"
#include "Asset/Cooked/MtlCookedData.h"
#include "Asset/Runtime/MaterialRenderResource.h"
#include "RHI/DynamicRHI.h"

using namespace Asset;

class ENGINE_API UMaterial : public UAsset
{
    DECLARE_RTTI(UMaterial, UAsset)

  public:
    bool LoadFromCooked(const FString& InAssetPath, std::shared_ptr<FMtlCookedData> InCookedData,
                        RHI::FDynamicRHI& InDynamicRHI);

    const std::shared_ptr<FMtlCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FMtlCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FMaterialRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FMaterialRenderResource> InRenderResource)
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

    bool IsValidLowLevel() const override;

  private:
    std::shared_ptr<FMtlCookedData>          CookedData;
    std::shared_ptr<FMaterialRenderResource> RenderResource;
};