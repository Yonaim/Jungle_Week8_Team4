#pragma once

#include "Engine/Asset/FontAtlas.h"
#include "TextRenderComponent.h"
#include "Engine/Component/Text/TextRenderComponent.h"

class AActor;

namespace Engine::Component
{
    class ENGINE_API UAtlasTextComponent : public UTextRenderComponent
    {
        DECLARE_RTTI(UAtlasTextComponent, UTextRenderComponent)

      public:
        UAtlasTextComponent();
        ~UAtlasTextComponent() override = default;

        const UFontAtlas* GetFontAsset() const { return FontAsset; }
        UFontAtlas*       GetFontAsset() { return FontAsset; }
        void              SetFontAsset(UFontAtlas* InFontAsset);

        const FFontAtlasRenderResource* GetFontRenderResource() const;
        FFontAtlasRenderResource*       GetFontRenderResource();
        const FFontAtlasRenderResource* GetFontResource() const { return GetFontRenderResource(); }
        FFontAtlasRenderResource*       GetFontResource() { return GetFontRenderResource(); }

        float GetTextScale() const { return UTextRenderComponent::GetTextScale(); }
        void  SetTextScale(float InTextScale);

        float GetLetterSpacing() const { return UTextRenderComponent::GetLetterSpacing(); }
        void  SetLetterSpacing(float InLetterSpacing);

        float GetLineSpacing() const { return UTextRenderComponent::GetLineSpacing(); }
        void  SetLineSpacing(float InLineSpacing);

        void            DescribeProperties(FComponentPropertyBuilder& Builder) override;
        virtual FMatrix GetRenderPlacementWorld(const AActor& InOwnerActor) const;
        virtual FVector GetRenderPlacementOffset(const AActor& InOwnerActor) const;

        EBasicMeshType GetBasicMeshType() const override;

      protected:
        Geometry::FAABB GetLocalAABB() const override { return {}; }
        const FFontResource* ResolveFontResourceForCollect() const override;

        UFontAtlas* FontAsset = nullptr;
    };
} // namespace Engine::Component
