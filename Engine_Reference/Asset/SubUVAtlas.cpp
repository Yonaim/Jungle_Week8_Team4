#include "Engine/Asset/SubUVAtlas.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Misc/Paths.h"

#include <filesystem>

REGISTER_CLASS(, USubUVAtlas)

namespace
{
    static FString BuildAssetNameFromPath(const FString& InAssetPath)
    {
        if (InAssetPath.empty())
        {
            return {};
        }

        const std::filesystem::path FilePath = FPaths::PathFromUtf8(InAssetPath);
        return FPaths::Utf8FromPath(FilePath.stem());
    }
} // namespace

bool USubUVAtlas::LoadFromCooked(const FString&                         InAssetPath,
                                 std::shared_ptr<FSubUVAtlasCookedData> InCookedData,
                                 RHI::FDynamicRHI&                      InDynamicRHI)
{
    if (InCookedData == nullptr)
    {
        UE_LOG(SubUVAtlas, ELogLevel::Error, "SubUV atlas asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    if (CookedData == InCookedData && GetAssetPath() == InAssetPath && RenderResource != nullptr)
    {
        SetLoaded(true);
        return true;
    }

    std::shared_ptr<FSubUVAtlasRenderResource> NewRenderResource =
        FSubUVAtlasRenderResource::Create(*InCookedData, InDynamicRHI);
    if (NewRenderResource == nullptr)
    {
        UE_LOG(SubUVAtlas, ELogLevel::Error, "SubUV atlas asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    SetAssetPath(InAssetPath);
    SetAssetName(BuildAssetNameFromPath(InAssetPath));
    SetCookedData(std::move(InCookedData));
    SetRenderResource(std::move(NewRenderResource));
    SetLoaded(true);
    UE_LOG(SubUVAtlas, ELogLevel::Debug, "SubUV atlas asset load succeeded: %s", InAssetPath.c_str());
    return true;
}


bool USubUVAtlas::IsValidLowLevel() const
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
