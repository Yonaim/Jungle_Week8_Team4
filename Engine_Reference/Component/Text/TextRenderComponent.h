#pragma once
#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/RenderAsset/FontResource.h"
#include "Renderer/Types/RenderItem.h"

namespace Engine::Component
{
    class ENGINE_API UTextRenderComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(UTextRenderComponent, UPrimitiveComponent)

      public:
        UTextRenderComponent() = default;
        ~UTextRenderComponent() override = default;

        const FString& GetText() const { return Text; }
        void           SetText(const FString& InText);

        bool GetBillboard() const { return bBillboard; }
        void SetBillboard(bool bInBillboard);

        const FVector& GetBillboardOffset() const { return BillboardOffset; }
        void           SetBillboardOffset(const FVector& InBillboardOffset);

        const FFontResource* GetFontResource() const { return FontResource; }
        const FFontResource* GetFontResource() { return FontResource; }
        void                 SetFontResource(const FFontResource* InFontResource);

        const FString& GetFontPath() const { return FontPath; }
        void           SetFontPath(const FString& InFontPath);

        float GetTextScale() const { return TextScale; }
        void  SetTextScale(float InTextScale);

        float GetLetterSpacing() const { return LetterSpacing; }
        void  SetLetterSpacing(float InLetterSpacing);

        float GetLineSpacing() const { return LineSpacing; }
        void  SetLineSpacing(float InLineSpacing);

        ETextLayoutMode GetLayoutMode() const { return LayoutMode; }
        void            SetLayoutMode(ETextLayoutMode InLayoutMode);

        void DescribeProperties(FComponentPropertyBuilder& Builder) override;

        EBasicMeshType GetBasicMeshType() const override;

        virtual FMatrix GetRenderPlacementWorld(const AActor& InOwnerActor) const;
        virtual FVector GetRenderPlacementOffset(const AActor& InOwnerActor) const;

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;

      protected:
        Geometry::FAABB GetLocalAABB() const override;
        void BuildMeshWithTextLayout(std::shared_ptr<FMeshData> InMeshData) const;

      protected:
        enum class EResolvedGlyphKind : uint8
        {
            Normal,
            QuestionFallback,
            Missing
        };

        struct FResolvedGlyph
        {
            const FFontGlyph*  Glyph = nullptr;
            EResolvedGlyphKind Kind = EResolvedGlyphKind::Missing;
        };

        struct FLaidOutGlyph
        {
            const FFontGlyph* Glyph = nullptr;

            float MinX = 0.0f;
            float MinY = 0.0f;
            float MaxX = 0.0f;
            float MaxY = 0.0f;

            bool   bSolidColorQuad = false;
            FColor SolidColor = FColor::White();
        };

        struct FTextLayout
        {
            TArray<FLaidOutGlyph> Glyphs;

            float MinX = 0.0f;
            float MinY = 0.0f;
            float MaxX = 0.0f;
            float MaxY = 0.0f;

            float GetWidth() const { return MaxX - MinX; }
            float GetHeight() const { return MaxY - MinY; }
            bool  HasGlyphs() const { return !Glyphs.empty(); }
            bool  IsValid() const { return HasGlyphs() && GetWidth() > 0.0f && GetHeight() > 0.0f; }
        };

        FResolvedGlyph ResolveGlyph(const FFontResource& InFont, uint32 InCodePoint) const;
        float          GetMissingGlyphAdvance(const FFontResource& InFont, float InLineHeight,
                                              float InScale) const;

        FTextLayout                  BuildTextLayout() const;
        virtual const FFontResource* ResolveFontResourceForCollect() const {return nullptr;}

    protected:
        bool CreateRenderCommand(FSceneRenderData& InRenderData, ESceneShowFlags InShowFlags,
                                 FRenderCommand& Command) const;
    protected:
        FString Text;
        FString FontPath;
        const FFontResource* FontResource = nullptr;

        float TextScale = 1.0f;
        float LetterSpacing = 0.0f;
        float LineSpacing = 0.0f;
        ETextLayoutMode LayoutMode = ETextLayoutMode::Natural;

        bool    bBillboard = false;
        FVector BillboardOffset = FVector(0.0f, 0.0f, 0.0f);

        mutable std::shared_ptr<FMeshData>       MeshData;
        mutable std::shared_ptr<UMaterial> Material;
    };
} // namespace Engine::Component