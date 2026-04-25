#include "Engine/Asset/FontAtlas.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Misc/Paths.h"

#include <filesystem>

REGISTER_CLASS(, UFontAtlas)

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
}

bool UFontAtlas::LoadFromCooked(const FString& InAssetPath,
                                std::shared_ptr<FFontAtlasCookedData> InCookedData,
                                RHI::FDynamicRHI& InDynamicRHI)
{
    if (InCookedData == nullptr)
    {
        UE_LOG(FontAsset, ELogLevel::Error, "Font atlas asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    if (CookedData == InCookedData && GetAssetPath() == InAssetPath && RenderResource != nullptr)
    {
        SetLoaded(true);
        return true;
    }

    std::shared_ptr<FFontAtlasRenderResource> NewRenderResource =
        FFontAtlasRenderResource::Create(*InCookedData, InDynamicRHI);
    if (NewRenderResource == nullptr)
    {
        UE_LOG(FontAsset, ELogLevel::Error, "Font atlas asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    SetAssetPath(InAssetPath);
    SetAssetName(BuildAssetNameFromPath(InAssetPath));
    SetCookedData(std::move(InCookedData));
    SetRenderResource(std::move(NewRenderResource));
    SetLoaded(true);
    UE_LOG(FontAsset, ELogLevel::Debug, "Font atlas asset load succeeded: %s", InAssetPath.c_str());
    return true;
}


bool UFontAtlas::IsValidLowLevel() const
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
