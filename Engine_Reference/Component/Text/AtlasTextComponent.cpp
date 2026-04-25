#include "AtlasTextComponent.h"

#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    UAtlasTextComponent::UAtlasTextComponent() 
    {
        FontPath = "/Content/Font/JetBrainsMono/JetBrainsMono_Medium.font.json";
    }

    void UAtlasTextComponent::SetFontAsset(UFontAtlas* InFontAsset)
    {
        if (FontAsset == InFontAsset)
        {
            return;
        }

        FontAsset = InFontAsset;
        SetFontResource((FontAsset != nullptr && FontAsset->GetRenderResource())
                            ? FontAsset->GetRenderResource().get()
                            : nullptr);
    }

    const FFontAtlasRenderResource* UAtlasTextComponent::GetFontRenderResource() const
    {
        return (FontAsset != nullptr && FontAsset->GetRenderResource())
                   ? FontAsset->GetRenderResource().get()
                   : nullptr;
    }

    FFontAtlasRenderResource* UAtlasTextComponent::GetFontRenderResource()
    {
        return (FontAsset != nullptr && FontAsset->GetRenderResource())
                   ? FontAsset->GetRenderResource().get()
                   : nullptr;
    }

    void UAtlasTextComponent::SetTextScale(float InTextScale)
    {
        UTextRenderComponent::SetTextScale(InTextScale);
    }

    void UAtlasTextComponent::SetLetterSpacing(float InLetterSpacing)
    {
        UTextRenderComponent::SetLetterSpacing(InLetterSpacing);
    }

    void UAtlasTextComponent::SetLineSpacing(float InLineSpacing)
    {
        UTextRenderComponent::SetLineSpacing(InLineSpacing);
    }

    void UAtlasTextComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UTextRenderComponent::DescribeProperties(Builder);
    }

    FMatrix UAtlasTextComponent::GetRenderPlacementWorld(const AActor& InOwnerActor) const
    {
        return InOwnerActor.GetWorldMatrix();
    }

    FVector UAtlasTextComponent::GetRenderPlacementOffset(const AActor& InOwnerActor) const
    {
        (void)InOwnerActor;
        return FVector::ZeroVector;
    }

    const FFontAtlasRenderResource* UAtlasTextComponent::ResolveFontResourceForCollect() const
    {
        return GetFontRenderResource();
    }

    EBasicMeshType UAtlasTextComponent::GetBasicMeshType() const { return EBasicMeshType::None; }

    REGISTER_CLASS(Engine::Component, UAtlasTextComponent)
} // namespace Engine::Component
