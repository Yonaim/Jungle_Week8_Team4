#include "PaperSpriteComponent.h"

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <filesystem>

#include "Core/Logging/LogMacros.h"
#include "Core/Geometry/Primitives/AABBUtility.h"
#include "Engine/Asset/Material.h"
#include "Engine/Asset/StaticMesh.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Game/Actor.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/Primitive/FMeshData.h"
#include "Renderer/RenderAsset/TextureResource.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SceneView.h"
#include "RHI/D3D11/D3D11Texture.h"

namespace Engine::Component
{
    namespace
    {
        static bool ReadStaticMeshVertexPosition(const UStaticMesh* InMeshAsset, uint32 VertexIndex,
                                                 FVector& OutPosition)
        {
            if (InMeshAsset == nullptr || !InMeshAsset->IsValidLowLevel())
            {
                return false;
            }

            const TArray<uint8>& VertexData = InMeshAsset->GetVerticesData();
            const uint32         VertexStride = InMeshAsset->GetVertexStride();
            const uint32         VertexCount = InMeshAsset->GetVerticesCount();
            if (VertexStride < sizeof(FVector) || VertexIndex >= VertexCount)
            {
                return false;
            }

            const size_t Offset = static_cast<size_t>(VertexIndex) * VertexStride;
            if (Offset + sizeof(FVector) > VertexData.size())
            {
                return false;
            }

            std::memcpy(&OutPosition, VertexData.data() + Offset, sizeof(FVector));
            return true;
        }
    } // namespace

    const FString& UPaperSpriteComponent::GetDefaultQuadMeshPath()
    {
        static const FString Path = "/Content/Mesh/Primitive/quad.obj";
        return Path;
    }

    void UPaperSpriteComponent::SetMeshAssetPath(const FString& InPath)
    {
        const FString& NewPath = InPath.empty() ? GetDefaultQuadMeshPath() : InPath;
        if (MeshPath == NewPath)
        {
            return;
        }

        MeshPath = NewPath;
        MeshAsset = nullptr;
        MeshData.reset();
        bBoundsDirty = true;
        UE_LOG(SpriteComponent, ELogLevel::Verbose, "Sprite mesh path changed: %s", MeshPath.c_str());
    }

    void UPaperSpriteComponent::SetMeshAsset(UStaticMesh* InMeshAsset)
    {
        if (MeshAsset == InMeshAsset)
        {
            return;
        }

        MeshAsset = InMeshAsset;
        MeshData.reset();
        bBoundsDirty = true;
        UE_LOG(SpriteComponent, ELogLevel::Debug, "Sprite mesh asset assigned: %s",
               MeshAsset ? MeshAsset->GetAssetPath().c_str() : "<null>");
    }

    void UPaperSpriteComponent::SetTextureAsset(UTexture* InTextureAsset)
    {
        if (TextureAsset == InTextureAsset)
        {
            return;
        }

        TextureAsset = InTextureAsset;
        Material = nullptr;
        bBoundsDirty = true;
        UE_LOG(SpriteComponent, ELogLevel::Debug, "Sprite texture asset assigned: %s",
               TextureAsset ? TextureAsset->GetAssetPath().c_str() : "<null>");
    }

    void UPaperSpriteComponent::SetTexturePath(const FString& InPath)
    {
        if (TexturePath == InPath)
        {
            return;
        }

        TexturePath = InPath;
        TextureAsset = nullptr;
        Material = nullptr;
        bBoundsDirty = true;
        UE_LOG(SpriteComponent, ELogLevel::Verbose, "Sprite texture path changed: %s", TexturePath.c_str());
    }

    const FTextureRenderResource* UPaperSpriteComponent::GetTextureRenderResource() const
    {
        return (TextureAsset != nullptr && TextureAsset->GetRenderResource())
                   ? TextureAsset->GetRenderResource().get()
                   : nullptr;
    }

    FTextureRenderResource* UPaperSpriteComponent::GetTextureRenderResource()
    {
        return (TextureAsset != nullptr && TextureAsset->GetRenderResource())
                   ? TextureAsset->GetRenderResource().get()
                   : nullptr;
    }

    void UPaperSpriteComponent::SetBillboard(bool bInBillboard)
    {
        if (bBillboard == bInBillboard)
        {
            return;
        }

        bBillboard = bInBillboard;
        bBoundsDirty = true;
        UE_LOG(SpriteComponent, ELogLevel::Verbose, "Sprite billboard changed: %d", bBillboard ? 1 : 0);
    }

    void UPaperSpriteComponent::SetBillboardOffset(const FVector& InBillboardOffset)
    {
        if (BillboardOffset == InBillboardOffset)
        {
            return;
        }

        BillboardOffset = InBillboardOffset;
        bBoundsDirty = true;
        UE_LOG(SpriteComponent, ELogLevel::Verbose,
               "Sprite billboard offset changed: (%.3f, %.3f, %.3f)",
               BillboardOffset.X, BillboardOffset.Y, BillboardOffset.Z);
    }

    void UPaperSpriteComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        UMeshComponent::DescribeProperties(Builder);

        FComponentPropertyOptions TexturePathOptions;
        TexturePathOptions.ExpectedAssetPathKind = EComponentAssetPathKind::TextureImage;

        Builder.AddAssetPath(
            "texture", L"Texture", [this]() { return GetTexturePath(); },
            [this](const FString& InPath) { SetTexturePath(InPath); }, TexturePathOptions);

        Builder.AddBool(
            "billboard", L"Billboard", [this]() { return GetBillboard(); },
            [this](bool bInValue) { SetBillboard(bInValue); });
    }

    void UPaperSpriteComponent::GetUVs(FVector2& OutUVMin, FVector2& OutUVMax) const
    {
        OutUVMin = FVector2(0.0f, 0.0f);
        OutUVMax = FVector2(1.0f, 1.0f);
    }

    FVector2 UPaperSpriteComponent::GetSpriteAspectScale() const
    {
        const FTextureRenderResource* TextureResource = GetTextureRenderResource();
        if (TextureResource == nullptr || TextureResource->Width <= 0 ||
            TextureResource->Height <= 0)
        {
            return FVector2(1.0f, 1.0f);
        }

        const float Width = static_cast<float>(TextureResource->Width);
        const float Height = static_cast<float>(TextureResource->Height);
        if (Width <= 0.0f || Height <= 0.0f)
        {
            return FVector2(1.0f, 1.0f);
        }

        if (Width >= Height)
        {
            return FVector2(1.0f, Height / Width);
        }

        return FVector2(Width / Height, 1.0f);
    }

    void UPaperSpriteComponent::EnsureDynamicQuadMeshData() const
    {
        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
        }

        FVector2 UVMin;
        FVector2 UVMax;
        GetUVs(UVMin, UVMax);

        MeshData->bIsDynamicMesh = true;
        MeshData->Topology = EMeshTopology::EMT_TriangleList;
        MeshData->VertexBuffer.reset();
        MeshData->IndexBuffer.reset();
        MeshData->Vertices = {
            {FVector(-1.0f, -1.0f, 0.0f), FVector(0, 0, 1), FColor::White(),
             FVector2(UVMin.X, UVMax.Y)},
            {FVector(-1.0f, 1.0f, 0.0f), FVector(0, 0, 1), FColor::White(),
             FVector2(UVMax.X, UVMax.Y)},
            {FVector(1.0f, 1.0f, 0.0f), FVector(0, 0, 1), FColor::White(),
             FVector2(UVMax.X, UVMin.Y)},
            {FVector(1.0f, -1.0f, 0.0f), FVector(0, 0, 1), FColor::White(),
             FVector2(UVMin.X, UVMin.Y)},
        };
        MeshData->Indices = {0, 2, 1, 0, 3, 2};
        MeshData->VertexBufferCount = static_cast<uint32>(MeshData->Vertices.size());
        MeshData->IndexBufferCount = static_cast<uint32>(MeshData->Indices.size());
    }

    void UPaperSpriteComponent::EnsureStaticMeshRenderData() const
    {
        if (MeshAsset == nullptr || !MeshAsset->IsValidLowLevel() ||
            MeshAsset->GetRenderResource() == nullptr)
        {
            EnsureDynamicQuadMeshData();
            return;
        }

        if (!MeshData)
        {
            MeshData = std::make_shared<FMeshData>();
        }

        MeshData->bIsDynamicMesh = false;
        MeshData->Topology = EMeshTopology::EMT_TriangleList;
        MeshData->Vertices.clear();
        MeshData->Indices.clear();
        MeshData->VertexBuffer = MeshAsset->GetRenderResource()->VertexBuffer;
        MeshData->IndexBuffer = MeshAsset->GetRenderResource()->IndexBuffer;
        MeshData->VertexBufferCount = MeshAsset->GetVerticesCount();
        MeshData->IndexBufferCount = MeshAsset->GetIndicesCount();
    }

    void UPaperSpriteComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                                  ESceneShowFlags   InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_Sprites))
        {
            return;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return;
        }

        EnsureStaticMeshRenderData();

        if (MeshData && MeshData->bIsDynamicMesh)
        {
            EnsureDynamicQuadMeshData();
        }

        if (TextureAsset)
        {
            if (!Material)
            {
                Material = std::make_shared<UMaterial>();
                Material->SetAssetName("M_Sprite_" + TextureAsset->GetAssetName());

                auto CookedData = std::make_shared<FMtlCookedData>();
                CookedData->Name = "M_Sprite_" + TextureAsset->GetAssetName();
                Material->SetCookedData(CookedData);
            }

            std::shared_ptr<FMaterialRenderResource> RenderResource = Material->GetRenderResource();
            if (!RenderResource)
            {
                RenderResource = std::make_shared<FMaterialRenderResource>();
                Material->SetRenderResource(RenderResource);
            }

            RenderResource->BaseColorTexture.reset();
            RenderResource->NormalTexture.reset();
            RenderResource->ORMTexture.reset();

            if (TextureAsset->GetRenderResource() && TextureAsset->GetRenderResource()->GetSRV())
            {
                RHI::FTextureDesc Desc;
                Desc.Width =
                    TextureAsset->GetCookedData() ? TextureAsset->GetCookedData()->Width : 0;
                Desc.Height =
                    TextureAsset->GetCookedData() ? TextureAsset->GetCookedData()->Height : 0;
                Desc.Format = RHI::EPixelFormat::RGBA32F;
                RenderResource->BaseColorTexture = std::make_shared<RHI::D3D11::FD3D11Texture2D>(
                    Desc, nullptr, TextureAsset->GetRenderResource()->GetSRV());
            }
        }
        else
        {
            Material = nullptr;
        }

        FRenderCommand Command;
        Command.MeshData = MeshData.get();
        Command.Material = Material ? Material.get() : FGeneralRenderer::GetDefaultSpriteMaterial();

        const FVector2 SpriteAspectScale = GetSpriteAspectScale();
        const FMatrix  ActorWorld = Actor->GetWorldMatrix();
        const FVector  SpriteOrigin = ActorWorld.GetOrigin() + BillboardOffset;

        if (bBillboard && OutRenderData.SceneView)
        {
            const FMatrix CameraWorld = OutRenderData.SceneView->GetViewMatrix().GetInverse();
            const FVector RightAxis = CameraWorld.GetRightVector();
            const FVector UpAxis = CameraWorld.GetUpVector();
            const FVector ForwardAxis = CameraWorld.GetForwardVector();
            const FVector WorldScale = Actor->GetScale();

            const FVector Row0 = UpAxis * (WorldScale.X * SpriteAspectScale.X);
            const FVector Row1 = RightAxis * (WorldScale.Y * SpriteAspectScale.Y);
            const FVector Row2 = -ForwardAxis * WorldScale.Z;

            Command.WorldMatrix.M[0][0] = Row0.X;
            Command.WorldMatrix.M[0][1] = Row0.Y;
            Command.WorldMatrix.M[0][2] = Row0.Z;
            Command.WorldMatrix.M[0][3] = 0.0f;
            Command.WorldMatrix.M[1][0] = Row1.X;
            Command.WorldMatrix.M[1][1] = Row1.Y;
            Command.WorldMatrix.M[1][2] = Row1.Z;
            Command.WorldMatrix.M[1][3] = 0.0f;
            Command.WorldMatrix.M[2][0] = Row2.X;
            Command.WorldMatrix.M[2][1] = Row2.Y;
            Command.WorldMatrix.M[2][2] = Row2.Z;
            Command.WorldMatrix.M[2][3] = 0.0f;
            Command.WorldMatrix.M[3][0] = SpriteOrigin.X;
            Command.WorldMatrix.M[3][1] = SpriteOrigin.Y;
            Command.WorldMatrix.M[3][2] = SpriteOrigin.Z;
            Command.WorldMatrix.M[3][3] = 1.0f;
        }
        else
        {
            Command.WorldMatrix = ActorWorld;
            Command.WorldMatrix.M[0][0] *= SpriteAspectScale.X;
            Command.WorldMatrix.M[0][1] *= SpriteAspectScale.X;
            Command.WorldMatrix.M[0][2] *= SpriteAspectScale.X;
            Command.WorldMatrix.M[1][0] *= SpriteAspectScale.Y;
            Command.WorldMatrix.M[1][1] *= SpriteAspectScale.Y;
            Command.WorldMatrix.M[1][2] *= SpriteAspectScale.Y;
            Command.WorldMatrix.M[3][0] += BillboardOffset.X;
            Command.WorldMatrix.M[3][1] += BillboardOffset.Y;
            Command.WorldMatrix.M[3][2] += BillboardOffset.Z;
        }

        Command.ObjectId = Actor->GetObjectId();
        Command.bDrawAABB = Actor->IsSelected();
        Command.WorldAABB = GetWorldAABB();
        Command.SetDefaultStates();
        Command.RasterizerOption.CullMode = D3D11_CULL_NONE;
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

        OutRenderData.RenderCommands.push_back(Command);
    }

    bool UPaperSpriteComponent::GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const
    {
        OutTriangles.clear();

        if (MeshAsset == nullptr || !MeshAsset->IsValidLowLevel())
        {
            return false;
        }

        const TArray<uint32>& Indices = MeshAsset->GetIndicesData();
        for (size_t i = 0; i + 2 < Indices.size(); i += 3)
        {
            FVector P0, P1, P2;
            if (!ReadStaticMeshVertexPosition(MeshAsset, Indices[i + 0], P0) ||
                !ReadStaticMeshVertexPosition(MeshAsset, Indices[i + 1], P1) ||
                !ReadStaticMeshVertexPosition(MeshAsset, Indices[i + 2], P2))
            {
                continue;
            }

            Geometry::FTriangle Triangle;
            Triangle.V0 = P0;
            Triangle.V1 = P1;
            Triangle.V2 = P2;
            OutTriangles.push_back(Triangle);
        }

        return !OutTriangles.empty();
    }

    Geometry::FAABB UPaperSpriteComponent::GetLocalAABB() const
    {
        if (MeshAsset != nullptr && MeshAsset->IsValidLowLevel())
        {
            return MeshAsset->GetAABB();
        }

        return Geometry::FAABB();
    }

    void UPaperSpriteComponent::UpdateBounds()
    {
        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            WorldAABB = Geometry::FAABB();
            return;
        }

        const Geometry::FAABB LocalAABB = GetLocalAABB();

        const FVector2 SpriteAspectScale = GetSpriteAspectScale();
        const FMatrix  ActorWorld = Actor->GetWorldMatrix();
        const FVector  SpriteOrigin = ActorWorld.GetOrigin() + BillboardOffset;

        FMatrix BoundsWorldMatrix = ActorWorld;

        if (bBillboard)
        {
            const FVector WorldScale = Actor->GetScale();

            FVector RightAxis(1.0f, 0.0f, 0.0f);
            FVector UpAxis(0.0f, 1.0f, 0.0f);
            FVector ForwardAxis(0.0f, 0.0f, 1.0f);

            if (ActorWorld.GetRightVector().SizeSquared() > 0.0f)
            {
                RightAxis = ActorWorld.GetRightVector().GetSafeNormal();
            }

            if (ActorWorld.GetUpVector().SizeSquared() > 0.0f)
            {
                UpAxis = ActorWorld.GetUpVector().GetSafeNormal();
            }

            if (ActorWorld.GetForwardVector().SizeSquared() > 0.0f)
            {
                ForwardAxis = ActorWorld.GetForwardVector().GetSafeNormal();
            }

            const FVector Row0 = UpAxis * (WorldScale.X * SpriteAspectScale.X);
            const FVector Row1 = RightAxis * (WorldScale.Y * SpriteAspectScale.Y);
            const FVector Row2 = ForwardAxis * WorldScale.Z;

            BoundsWorldMatrix.M[0][0] = Row0.X;
            BoundsWorldMatrix.M[0][1] = Row0.Y;
            BoundsWorldMatrix.M[0][2] = Row0.Z;
            BoundsWorldMatrix.M[0][3] = 0.0f;

            BoundsWorldMatrix.M[1][0] = Row1.X;
            BoundsWorldMatrix.M[1][1] = Row1.Y;
            BoundsWorldMatrix.M[1][2] = Row1.Z;
            BoundsWorldMatrix.M[1][3] = 0.0f;

            BoundsWorldMatrix.M[2][0] = Row2.X;
            BoundsWorldMatrix.M[2][1] = Row2.Y;
            BoundsWorldMatrix.M[2][2] = Row2.Z;
            BoundsWorldMatrix.M[2][3] = 0.0f;

            BoundsWorldMatrix.M[3][0] = SpriteOrigin.X;
            BoundsWorldMatrix.M[3][1] = SpriteOrigin.Y;
            BoundsWorldMatrix.M[3][2] = SpriteOrigin.Z;
            BoundsWorldMatrix.M[3][3] = 1.0f;
        }
        else
        {
            BoundsWorldMatrix.M[0][0] *= SpriteAspectScale.X;
            BoundsWorldMatrix.M[0][1] *= SpriteAspectScale.X;
            BoundsWorldMatrix.M[0][2] *= SpriteAspectScale.X;

            BoundsWorldMatrix.M[1][0] *= SpriteAspectScale.Y;
            BoundsWorldMatrix.M[1][1] *= SpriteAspectScale.Y;
            BoundsWorldMatrix.M[1][2] *= SpriteAspectScale.Y;

            BoundsWorldMatrix.M[3][0] += BillboardOffset.X;
            BoundsWorldMatrix.M[3][1] += BillboardOffset.Y;
            BoundsWorldMatrix.M[3][2] += BillboardOffset.Z;
        }

        WorldAABB = Geometry::TransformAABB(LocalAABB, BoundsWorldMatrix);
    }

    REGISTER_CLASS(Engine::Component, UPaperSpriteComponent)
} // namespace Engine::Component