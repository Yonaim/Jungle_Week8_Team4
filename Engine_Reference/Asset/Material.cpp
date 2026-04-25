#include "Engine/Asset/Material.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Misc/Paths.h"

#include <filesystem>

REGISTER_CLASS(, UMaterial)

namespace
{
    static FString BuildAssetNameFromPath(const FString&                         InAssetPath,
                                          const std::shared_ptr<FMtlCookedData>& InCookedData)
    {
        if (InCookedData != nullptr && !InCookedData->Name.empty())
        {
            return InCookedData->Name;
        }

        if (InAssetPath.empty())
        {
            return {};
        }

        const std::filesystem::path FilePath = FPaths::PathFromUtf8(InAssetPath);
        return FPaths::Utf8FromPath(FilePath.stem());
    }
} // namespace

bool UMaterial::LoadFromCooked(const FString&                  InAssetPath,
                               std::shared_ptr<FMtlCookedData> InCookedData,
                               RHI::FDynamicRHI&               InDynamicRHI)
{
    if (InCookedData == nullptr || !InCookedData->IsValid())
    {
        UE_LOG(MaterialAsset, ELogLevel::Error, "Material asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    if (CookedData == InCookedData && GetAssetPath() == InAssetPath && RenderResource != nullptr)
    {
        SetLoaded(true);
        return true;
    }

    std::shared_ptr<FMaterialRenderResource> NewRenderResource =
        FMaterialRenderResource::Create(*InCookedData, InDynamicRHI);
    if (NewRenderResource == nullptr)
    {
        UE_LOG(MaterialAsset, ELogLevel::Error, "Material asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    SetAssetPath(InAssetPath);
    SetAssetName(BuildAssetNameFromPath(InAssetPath, InCookedData));
    SetCookedData(std::move(InCookedData));
    SetRenderResource(std::move(NewRenderResource));
    SetLoaded(true);
    UE_LOG(MaterialAsset, ELogLevel::Debug, "Material asset load succeeded: %s", InAssetPath.c_str());
    return true;
}

bool UMaterial::IsValidLowLevel() const
{
    if (CookedData == nullptr)
    {
        return false;
    }

    if (!CookedData->IsValid())
    {
        return false;
    }

    if (RenderResource == nullptr)
    {
        return false;
    }

    return true;
}
