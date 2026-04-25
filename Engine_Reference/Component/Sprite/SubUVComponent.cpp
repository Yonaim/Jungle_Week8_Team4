#include "SubUVComponent.h"

#include "Engine/Component/Core/ComponentProperty.h"
#include <algorithm>

#include "Renderer/RenderAsset/SubUVAtlasResource.h"

namespace Engine::Component
{
    void USubUVComponent::SetSubUVAtlasAsset(USubUVAtlas* InAsset)
    {
        if (SubUVAtlasAsset == InAsset)
        {
            return;
        }

        SubUVAtlasAsset = InAsset;
        SetFrameIndex(FrameIndex);
    }

    void USubUVComponent::SetSubUVAtlasPath(const FString& InPath)
    {
        if (SubUVAtlasPath == InPath)
        {
            return;
        }

        SubUVAtlasPath = InPath;
        SubUVAtlasAsset = nullptr;
        SetFrameIndex(FrameIndex);
    }

    const FSubUVAtlasRenderResource* USubUVComponent::GetSubUVAtlasRenderResource() const
    {
        return (SubUVAtlasAsset != nullptr && SubUVAtlasAsset->GetRenderResource())
                   ? SubUVAtlasAsset->GetRenderResource().get()
                   : nullptr;
    }

    FSubUVAtlasRenderResource* USubUVComponent::GetSubUVAtlasRenderResource()
    {
        return (SubUVAtlasAsset != nullptr && SubUVAtlasAsset->GetRenderResource())
                   ? SubUVAtlasAsset->GetRenderResource().get()
                   : nullptr;
    }

    void USubUVComponent::SetFrameIndex(int32 InFrameIndex)
    {
        FrameIndex = (InFrameIndex >= 0) ? InFrameIndex : 0;

        const int32 FrameCount = GetFrameCount();
        if (FrameCount > 0 && FrameIndex >= FrameCount)
        {
            FrameIndex = FrameCount - 1;
        }
    }

    int32 USubUVComponent::GetFrameCount() const
    {
        const FSubUVAtlasRenderResource* Resource = GetSubUVAtlasRenderResource();
        if (Resource != nullptr)
        {
            if (!Resource->Frames.empty())
            {
                return static_cast<int32>(Resource->Frames.size());
            }

            if (Resource->Info.FrameCount > 0)
            {
                return static_cast<int32>(Resource->Info.FrameCount);
            }
        }

        return AtlasRows * AtlasColumns;
    }


    void USubUVComponent::GetUVs(FVector2& OutUVMin, FVector2& OutUVMax) const
    {
        const FSubUVAtlasRenderResource* Resource = GetSubUVAtlasRenderResource();
        if (Resource != nullptr && !Resource->Frames.empty() &&
            FrameIndex >= 0 && static_cast<size_t>(FrameIndex) < Resource->Frames.size())
        {
            const auto& Frame = Resource->Frames[FrameIndex];
            OutUVMin = FVector2(Frame.X, Frame.Y);
            OutUVMax = FVector2(Frame.X + Frame.Width, Frame.Y + Frame.Height);
            return;
        }

        const int32 SafeRows = std::max(1, AtlasRows);
        const int32 SafeColumns = std::max(1, AtlasColumns);
        const int32 SafeFrameCount = SafeRows * SafeColumns;
        const int32 SafeFrameIndex = (SafeFrameCount > 0)
                                        ? std::clamp(FrameIndex, 0, SafeFrameCount - 1)
                                        : 0;

        const int32 Row = SafeFrameIndex / SafeColumns;
        const int32 Column = SafeFrameIndex % SafeColumns;

        const float USize = 1.0f / static_cast<float>(SafeColumns);
        const float VSize = 1.0f / static_cast<float>(SafeRows);

        OutUVMin = FVector2(static_cast<float>(Column) * USize, static_cast<float>(Row) * VSize);
        OutUVMax = FVector2(OutUVMin.X + USize, OutUVMin.Y + VSize);
    }

    void USubUVComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UAtlasComponent::DescribeProperties(Builder);

        FComponentPropertyOptions AtlasPathOptions;
        AtlasPathOptions.ExpectedAssetPathKind = EComponentAssetPathKind::TextureAtlasFile;

        FComponentPropertyOptions IntOptions;
        IntOptions.DragSpeed = 1.0f;

        Builder.AddAssetPath(
            "subuv_atlas", L"SubUV Atlas", [this]() { return GetSubUVAtlasPath(); },
            [this](const FString& InPath) { SetSubUVAtlasPath(InPath); }, AtlasPathOptions);

        Builder.AddInt(
            "frame_index", L"Frame Index", [this]() { return GetFrameIndex(); },
            [this](int32 InValue) { SetFrameIndex(InValue); }, IntOptions);
    }

    REGISTER_CLASS(Engine::Component, USubUVComponent)
} // namespace Engine::Component
