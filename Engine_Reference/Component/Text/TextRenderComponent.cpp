#include "TextRenderComponent.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Game/Actor.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SceneView.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Engine/Asset/Material.h"
#include "Renderer/Types/RenderDebugColors.h"
#include "RHI/D3D11/D3D11Texture.h"

#include <algorithm>

namespace
{
    constexpr float DefaultLineHeight = 16.0f;
    constexpr uint32 Utf8ReplacementCodePoint = static_cast<uint32>('?');

    TArray<uint32> DecodeUtf8CodePoints(const FString& InText)
    {
        TArray<uint32> CodePoints;
        CodePoints.reserve(InText.size());

        const uint8* Bytes = reinterpret_cast<const uint8*>(InText.data());
        const size_t ByteCount = InText.size();

        size_t Index = 0;
        while (Index < ByteCount)
        {
            const uint8 Lead = Bytes[Index];

            if (Lead <= 0x7F)
            {
                CodePoints.push_back(static_cast<uint32>(Lead));
                ++Index;
                continue;
            }

            uint32 CodePoint = Utf8ReplacementCodePoint;
            size_t SequenceLength = 1;

            auto IsContinuationByte =
                [Bytes, ByteCount](size_t InIndex)
            {
                return InIndex < ByteCount && (Bytes[InIndex] & 0xC0) == 0x80;
            };

            if ((Lead & 0xE0) == 0xC0)
            {
                SequenceLength = 2;
                if (IsContinuationByte(Index + 1))
                {
                    const uint32 Candidate =
                        ((static_cast<uint32>(Lead & 0x1F)) << 6) |
                        static_cast<uint32>(Bytes[Index + 1] & 0x3F);

                    if (Candidate >= 0x80)
                    {
                        CodePoint = Candidate;
                    }
                }
            }
            else if ((Lead & 0xF0) == 0xE0)
            {
                SequenceLength = 3;
                if (IsContinuationByte(Index + 1) && IsContinuationByte(Index + 2))
                {
                    const uint32 Candidate =
                        ((static_cast<uint32>(Lead & 0x0F)) << 12) |
                        ((static_cast<uint32>(Bytes[Index + 1] & 0x3F)) << 6) |
                        static_cast<uint32>(Bytes[Index + 2] & 0x3F);

                    if (Candidate >= 0x800 && !(Candidate >= 0xD800 && Candidate <= 0xDFFF))
                    {
                        CodePoint = Candidate;
                    }
                }
            }
            else if ((Lead & 0xF8) == 0xF0)
            {
                SequenceLength = 4;
                if (IsContinuationByte(Index + 1) && IsContinuationByte(Index + 2) &&
                    IsContinuationByte(Index + 3))
                {
                    const uint32 Candidate =
                        ((static_cast<uint32>(Lead & 0x07)) << 18) |
                        ((static_cast<uint32>(Bytes[Index + 1] & 0x3F)) << 12) |
                        ((static_cast<uint32>(Bytes[Index + 2] & 0x3F)) << 6) |
                        static_cast<uint32>(Bytes[Index + 3] & 0x3F);

                    if (Candidate >= 0x10000 && Candidate <= 0x10FFFF)
                    {
                        CodePoint = Candidate;
                    }
                }
            }

            CodePoints.push_back(CodePoint);
            Index += SequenceLength;
        }

        return CodePoints;
    }
}

namespace Engine::Component
{
    void UTextRenderComponent::SetText(const FString& InText) 
    { 
        if (Text != InText)
        {
            Text = InText; 
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetBillboard(bool bInBillboard) { bBillboard = bInBillboard; }

    void UTextRenderComponent::SetBillboardOffset(const FVector& InBillboardOffset)
    {
        BillboardOffset = InBillboardOffset;
    }

    void UTextRenderComponent::SetFontResource(const FFontResource* InFontResource)
    {
        if (FontResource != InFontResource)
        {
            FontResource = InFontResource;
            MeshData = nullptr;
            Material = nullptr;
        }
    }

    void UTextRenderComponent::SetFontPath(const FString& InFontPath)
    {
        if (FontPath != InFontPath)
        {
            FontPath = InFontPath;
            FontResource = nullptr;
            MeshData = nullptr;
            Material = nullptr;
        }
    }

    void UTextRenderComponent::SetTextScale(float InTextScale) 
    { 
        if (TextScale != InTextScale)
        {
            TextScale = InTextScale; 
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetLetterSpacing(float InLetterSpacing)
    {
        if (LetterSpacing != InLetterSpacing)
        {
            LetterSpacing = InLetterSpacing;
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetLineSpacing(float InLineSpacing) 
    { 
        if (LineSpacing != InLineSpacing)
        {
            LineSpacing = InLineSpacing; 
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::SetLayoutMode(ETextLayoutMode InLayoutMode)
    {
        if (LayoutMode != InLayoutMode)
        {
            LayoutMode = InLayoutMode;
            MeshData = nullptr;
        }
    }

    void UTextRenderComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UPrimitiveComponent::DescribeProperties(Builder);

        FComponentPropertyOptions FontPathOptions;
        FontPathOptions.ExpectedAssetPathKind = EComponentAssetPathKind::FontFile;

        Builder.AddString(
            "text", L"Text", [this]() { return GetText(); },
            [this](const FString& InValue) { SetText(InValue); });

        Builder.AddFloat(
            "text_scale", L"Text Scale", [this]() { return GetTextScale(); },
            [this](float InValue) { SetTextScale(InValue); });

        Builder.AddFloat(
            "letter_spacing", L"Letter Spacing", [this]() { return GetLetterSpacing(); },
            [this](float InValue) { SetLetterSpacing(InValue); });

        Builder.AddFloat(
            "line_spacing", L"Line Spacing", [this]() { return GetLineSpacing(); },
            [this](float InValue) { SetLineSpacing(InValue); });

        Builder.AddAssetPath(
            "font_path", L"Font Path", [this]() { return GetFontPath(); },
            [this](const FString& InValue) { SetFontPath(InValue); }, FontPathOptions);

        Builder.AddVector3(
            "billboard_offset", L"Billboard Offset", [this]() { return GetBillboardOffset(); },
            [this](const FVector& InValue) { SetBillboardOffset(InValue); });
            
        Builder.AddBool(
            "billboard", L"Billboard", [this]() { return GetBillboard(); },
            [this](bool bInValue) { SetBillboard(bInValue); });
    }


    EBasicMeshType UTextRenderComponent::GetBasicMeshType() const { return EBasicMeshType::Quad; }

    FMatrix UTextRenderComponent::GetRenderPlacementWorld(const AActor& InOwnerActor) const
    {
        return InOwnerActor.GetWorldMatrix();
    }

    FVector UTextRenderComponent::GetRenderPlacementOffset(const AActor& InOwnerActor) const
    {
        (void)InOwnerActor;
        return FVector::ZeroVector;
    }

    UTextRenderComponent::FResolvedGlyph UTextRenderComponent::ResolveGlyph(const FFontResource& InFont, uint32 InCodePoint) const
    {
        if (const FFontGlyph* Glyph = InFont.FindGlyph(InCodePoint))
        {
            if (Glyph->IsValid())
            {
                return {Glyph, EResolvedGlyphKind::Normal};
            }
        }

        if (const FFontGlyph* QuestionGlyph = InFont.FindGlyph(static_cast<uint32>('?')))
        {
            if (QuestionGlyph->IsValid())
            {
                return {QuestionGlyph, EResolvedGlyphKind::QuestionFallback};
            }
        }

        return {nullptr, EResolvedGlyphKind::Missing};
    }

    float UTextRenderComponent::GetMissingGlyphAdvance(const FFontResource& InFont, float InLineHeight, float InScale) const
    {
        if (const FFontGlyph* SpaceGlyph = InFont.FindGlyph(static_cast<uint32>(' ')))
        {
            if (SpaceGlyph->IsValid())
            {
                return static_cast<float>(SpaceGlyph->XAdvance) * InScale;
            }
        }

        return InLineHeight * 0.5f * InScale;
    }

    UTextRenderComponent::FTextLayout UTextRenderComponent::BuildTextLayout() const
    {
        FTextLayout Layout;

        if (FontResource == nullptr || Text.empty())
        {
            return Layout;
        }

        const float RawLineHeight = (FontResource->Common.LineHeight > 0)
                                        ? static_cast<float>(FontResource->Common.LineHeight)
                                        : DefaultLineHeight;

        const float UnitScale =
            (RawLineHeight > 0.0f) ? (1.0f / RawLineHeight) : (1.0f / DefaultLineHeight);

        const float Scale = (TextScale > 0.0f) ? (TextScale * UnitScale) : UnitScale;
        const float LetterSpacingVal = LetterSpacing * UnitScale;
        const float LineSpacingVal = LineSpacing * UnitScale;

        const float MissingAdvance = GetMissingGlyphAdvance(*FontResource, RawLineHeight, Scale);
        const float MissingWidth = MissingAdvance;
        const float MissingHeight = RawLineHeight * Scale;

        float PenX = 0.0f;
        float PenY = 0.0f;
        bool  bHasBounds = false;

        const TArray<uint32> CodePoints = DecodeUtf8CodePoints(Text);

        for (uint32 CodePoint : CodePoints)
        {
            if (CodePoint == static_cast<uint32>('\r'))
            {
                continue;
            }

            if (CodePoint == static_cast<uint32>('\n'))
            {
                PenX = 0.0f;
                PenY += RawLineHeight * Scale + LineSpacingVal;
                continue;
            }

            if (CodePoint == static_cast<uint32>(' '))
            {
                float SpaceAdvance = 0.0f;

                if (const FFontGlyph* SpaceGlyph =
                        FontResource->FindGlyph(static_cast<uint32>(' ')))
                {
                    if (SpaceGlyph->IsValid())
                    {
                        SpaceAdvance = static_cast<float>(SpaceGlyph->XAdvance) * Scale;
                    }
                }

                if (SpaceAdvance <= 0.0f)
                {
                    SpaceAdvance = RawLineHeight * 0.25f * Scale;
                }

                PenX += SpaceAdvance;
                continue;
            }

            const FResolvedGlyph Resolved = ResolveGlyph(*FontResource, CodePoint);

            FLaidOutGlyph OutGlyph;

            if (Resolved.Kind == EResolvedGlyphKind::Missing)
            {
                OutGlyph.Glyph = nullptr;
                OutGlyph.bSolidColorQuad = true;
                OutGlyph.SolidColor = RenderDebugColors::MissingGlyph;

                OutGlyph.MinX = PenX;
                OutGlyph.MinY = PenY;
                OutGlyph.MaxX = PenX + MissingWidth;
                OutGlyph.MaxY = PenY + MissingHeight;

                PenX += MissingAdvance + LetterSpacingVal;
            }
            else
            {
                const FFontGlyph* Glyph = Resolved.Glyph;

                OutGlyph.Glyph = Glyph;
                OutGlyph.bSolidColorQuad = false;
                OutGlyph.MinX = PenX + static_cast<float>(Glyph->XOffset) * Scale;
                OutGlyph.MinY = PenY + static_cast<float>(Glyph->YOffset) * Scale;
                OutGlyph.MaxX = OutGlyph.MinX + static_cast<float>(Glyph->Width) * Scale;
                OutGlyph.MaxY = OutGlyph.MinY + static_cast<float>(Glyph->Height) * Scale;

                PenX += Scale * static_cast<float>(Glyph->XAdvance);
                PenX += LetterSpacingVal;
            }

            Layout.Glyphs.push_back(OutGlyph);

            if (!bHasBounds)
            {
                Layout.MinX = OutGlyph.MinX;
                Layout.MinY = OutGlyph.MinY;
                Layout.MaxX = OutGlyph.MaxX;
                Layout.MaxY = OutGlyph.MaxY;
                bHasBounds = true;
            }
            else
            {
                Layout.MinX = std::min(Layout.MinX, OutGlyph.MinX);
                Layout.MinY = std::min(Layout.MinY, OutGlyph.MinY);
                Layout.MaxX = std::max(Layout.MaxX, OutGlyph.MaxX);
                Layout.MaxY = std::max(Layout.MaxY, OutGlyph.MaxY);
            }
        }

        return Layout;
    }
    
    bool UTextRenderComponent::CreateRenderCommand(FSceneRenderData& InRenderData, ESceneShowFlags InShowFlags, FRenderCommand& Command) const
    {
        const FFontResource* ResolvedFontResource = ResolveFontResourceForCollect();
        if (FontResource != ResolvedFontResource)
        {
            const_cast<UTextRenderComponent*>(this)->SetFontResource(ResolvedFontResource);
        }

        if (Text.empty() || FontResource == nullptr)
        {
            return true;
        }

        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_BillboardText) && bBillboard)
        {
            return true;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return true;
        }

        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
            MeshData->bIsDynamicMesh = true;
            MeshData->Topology = EMeshTopology::EMT_TriangleList;

            FTextLayout Layout = BuildTextLayout();
            BuildMeshWithTextLayout(MeshData);
        }

        if (!Material && FontResource)
        {
            Material = std::make_shared<UMaterial>();
            Material->SetAssetName("M_Text");
            auto CookedData = std::make_shared<FMtlCookedData>();
            CookedData->Name = "M_Text";
            Material->SetCookedData(CookedData);

            auto RenderResource = std::make_shared<FMaterialRenderResource>();
            if (FontResource->AtlasTexture)
            {
                RenderResource->BaseColorTexture = FontResource->AtlasTexture;
            }
            Material->SetRenderResource(RenderResource);
        }

        Command.MeshData = MeshData.get();
        Command.Material = Material ? Material.get() : FGeneralRenderer::GetDefaultSpriteMaterial();
        
        const FMatrix PlacementWorld = GetRenderPlacementWorld(*Actor);
        const FVector PlacementOffset = GetRenderPlacementOffset(*Actor);
        FVector       Origin = PlacementWorld.GetOrigin() + PlacementOffset + BillboardOffset;

        if (bBillboard && InRenderData.SceneView)
        {
            const FMatrix CameraWorld = InRenderData.SceneView->GetViewMatrix().GetInverse();
            FVector       RightAxis = CameraWorld.GetRightVector();
            FVector       UpAxis = CameraWorld.GetUpVector();
            FVector       ForwardAxis = CameraWorld.GetForwardVector();

            const FVector WorldScale = Actor->GetScale();
            FVector       Row0 = UpAxis * WorldScale.X;
            FVector       Row1 = RightAxis * WorldScale.Y;
            FVector       Row2 = -ForwardAxis;

            Command.WorldMatrix.M[0][0] = Row0.X; Command.WorldMatrix.M[0][1] = Row0.Y; Command.WorldMatrix.M[0][2] = Row0.Z; Command.WorldMatrix.M[0][3] = 0.0f;
            Command.WorldMatrix.M[1][0] = Row1.X; Command.WorldMatrix.M[1][1] = Row1.Y; Command.WorldMatrix.M[1][2] = Row1.Z; Command.WorldMatrix.M[1][3] = 0.0f;
            Command.WorldMatrix.M[2][0] = Row2.X; Command.WorldMatrix.M[2][1] = Row2.Y; Command.WorldMatrix.M[2][2] = Row2.Z; Command.WorldMatrix.M[2][3] = 0.0f;
            Command.WorldMatrix.M[3][0] = Origin.X; Command.WorldMatrix.M[3][1] = Origin.Y; Command.WorldMatrix.M[3][2] = Origin.Z; Command.WorldMatrix.M[3][3] = 1.0f;
        }
        else
        {
            Command.WorldMatrix = PlacementWorld;
            Command.WorldMatrix.M[3][0] += PlacementOffset.X + BillboardOffset.X;
            Command.WorldMatrix.M[3][1] += PlacementOffset.Y + BillboardOffset.Y;
            Command.WorldMatrix.M[3][2] += PlacementOffset.Z + BillboardOffset.Z;
        }

        Command.ObjectId = Actor->GetObjectId();
        Command.bDrawAABB = Actor->IsSelected();
        Command.WorldAABB = GetWorldAABB();
        Command.SetDefaultStates();
        // Text needs alpha blending
        Command.BlendOption.BlendEnable = true;
        Command.BlendOption.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        Command.BlendOption.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        Command.BlendOption.BlendOp = D3D11_BLEND_OP_ADD;
        Command.BlendOption.SrcBlendAlpha = D3D11_BLEND_ONE;
        Command.BlendOption.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        Command.BlendOption.BlendOpAlpha = D3D11_BLEND_OP_ADD;

        Command.SetStates(Command.Material, MeshData->Topology);

        Command.bIsVisible = Actor->IsVisible();
        Command.bIsPickable = Actor->IsPickable();
        Command.bIsSelected = Actor->IsSelected();
        return false;
    }

    void UTextRenderComponent::CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const
    {
        FRenderCommand Command;
        if (CreateRenderCommand(OutRenderData, InShowFlags, Command)) 
            return;

        OutRenderData.RenderCommands.push_back(Command);
    }

    Geometry::FAABB UTextRenderComponent::GetLocalAABB() const
    {
        if (MeshData)
        {
            return Geometry::FAABB(MeshData->GetMinCoord(), MeshData->GetMaxCoord());
        }

        FTextLayout Layout = BuildTextLayout();
        if (Layout.IsValid())
        {
            float CenterX = (Layout.MinX + Layout.MaxX) * 0.5f;
            float CenterY = (Layout.MinY + Layout.MaxY) * 0.5f;
            return Geometry::FAABB(
                FVector(Layout.MinX - CenterX, CenterY - Layout.MaxY, 0.0f),
                FVector(Layout.MaxX - CenterX, CenterY - Layout.MinY, 0.0f)
            );
        }

        return {};
    }

    void UTextRenderComponent::BuildMeshWithTextLayout(std::shared_ptr<FMeshData> InMeshData) const
    {
        if (InMeshData == nullptr || FontResource == nullptr)
        {
            return;
        }

        const FTextLayout Layout = BuildTextLayout();
        if (!Layout.IsValid())
        {
            InMeshData->Vertices.clear();
            InMeshData->Indices.clear();
            InMeshData->VertexBufferCount = 0;
            InMeshData->IndexBufferCount = 0;
            return;
        }

        const float CenterX = (Layout.MinX + Layout.MaxX) * 0.5f;
        const float CenterY = (Layout.MinY + Layout.MaxY) * 0.5f;

        const float ScaleW = (FontResource->Common.ScaleW > 0)
                                 ? static_cast<float>(FontResource->Common.ScaleW)
                                 : 1.0f;
        const float ScaleH = (FontResource->Common.ScaleH > 0)
                                 ? static_cast<float>(FontResource->Common.ScaleH)
                                 : 1.0f;

        const uint32 GlyphCount = static_cast<uint32>(Layout.Glyphs.size());
        InMeshData->Vertices.clear();
        InMeshData->Indices.clear();
        InMeshData->Vertices.reserve(GlyphCount * 4);
        InMeshData->Indices.reserve(GlyphCount * 6);

        for (const FLaidOutGlyph& LaidOutGlyph : Layout.Glyphs)
        {
            const uint32 VertexOffset = static_cast<uint32>(InMeshData->Vertices.size());

            const float X_left = LaidOutGlyph.MinX - CenterX;
            const float X_right = LaidOutGlyph.MaxX - CenterX;
            const float Y_top = CenterY - LaidOutGlyph.MinY;
            const float Y_bottom = CenterY - LaidOutGlyph.MaxY;

            float  U_min = 0.0f, V_min = 0.0f, U_max = 0.0f, V_max = 0.0f;
            FColor GlyphColor = FColor::White();

            if (LaidOutGlyph.bSolidColorQuad)
            {
                GlyphColor = LaidOutGlyph.SolidColor;
                U_min = 0.0f; V_min = 0.0f; U_max = 1.0f; V_max = 1.0f;
            }
            else if (LaidOutGlyph.Glyph)
            {
                const FFontGlyph* Glyph = LaidOutGlyph.Glyph;
                U_min = static_cast<float>(Glyph->X) / ScaleW;
                V_min = static_cast<float>(Glyph->Y) / ScaleH;
                U_max = static_cast<float>(Glyph->X + Glyph->Width) / ScaleW;
                V_max = static_cast<float>(Glyph->Y + Glyph->Height) / ScaleH;
            }

            const FVector Normal(0.0f, 0.0f, 1.0f);

            // TL, TR, BR, BL (Clockwise winding for front face in Left-Handed space)
            InMeshData->Vertices.push_back(
                {FVector(Y_top, X_left, 0.0f), Normal, GlyphColor, FVector2(U_min, V_min)});
            InMeshData->Vertices.push_back(
                {FVector(Y_top, X_right, 0.0f), Normal, GlyphColor, FVector2(U_max, V_min)});
            InMeshData->Vertices.push_back(
                {FVector(Y_bottom, X_right, 0.0f), Normal, GlyphColor, FVector2(U_max, V_max)});
            InMeshData->Vertices.push_back(
                {FVector(Y_bottom, X_left, 0.0f), Normal, GlyphColor, FVector2(U_min, V_max)});

            InMeshData->Indices.push_back(VertexOffset + 0);
            InMeshData->Indices.push_back(VertexOffset + 1);
            InMeshData->Indices.push_back(VertexOffset + 2);
            InMeshData->Indices.push_back(VertexOffset + 0);
            InMeshData->Indices.push_back(VertexOffset + 2);
            InMeshData->Indices.push_back(VertexOffset + 3);
        }

        InMeshData->VertexBufferCount = static_cast<uint32>(InMeshData->Vertices.size());
        InMeshData->IndexBufferCount = static_cast<uint32>(InMeshData->Indices.size());
        InMeshData->UpdateLocalBound();
    }

    REGISTER_CLASS(Engine::Component, UTextRenderComponent)
} // namespace Engine::Component
