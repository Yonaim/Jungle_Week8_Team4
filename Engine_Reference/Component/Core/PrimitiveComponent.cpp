#include "PrimitiveComponent.h"
#include "Core/Logging/LogMacros.h"

#include "ComponentProperty.h"
#include "Core/Geometry/Primitives/AABBUtility.h"
#include "Core/Misc/BitMaskEnum.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/Types/RenderItem.h"
#include "Renderer/Types/BasicMeshType.h"
#include "Engine/Game/Actor.h"

#include <cfloat>

namespace Engine::Component
{
    const FColor& UPrimitiveComponent::GetColor() const { return Color; }

    void UPrimitiveComponent::SetColor(const FColor& NewColor)
    {
        if (Color == NewColor)
        {
            return;
        }

        Color = NewColor;
        UE_LOG(PrimitiveComponent, ELogLevel::Verbose,
               "Color updated on %s (owner=%s)", GetTypeName(),
               GetOwnerActor() ? GetOwnerActor()->GetTypeName() : "<none>");
    }

    const Geometry::FAABB& UPrimitiveComponent::GetWorldAABB() const
    {
        if (bBoundsDirty)
        {
            const_cast<UPrimitiveComponent*>(this)->UpdateBounds();
            const_cast<UPrimitiveComponent*>(this)->bBoundsDirty = false;
        }

        return WorldAABB;
    }

    bool UPrimitiveComponent::GetWorldAABB(Geometry::FAABB& OutWorldAABB) const
    {
        OutWorldAABB = GetWorldAABB();
        return true;
    }

    bool UPrimitiveComponent::GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const
    {
        OutTriangles.clear();

        if (RenderCommand.MeshData == nullptr)
        {
            return false;
        }

        if (RenderCommand.MeshData->Topology != EMeshTopology::EMT_TriangleList)
        {
            return false;
        }

        const auto& Vertices = RenderCommand.MeshData->Vertices;
        const auto& Indices = RenderCommand.MeshData->Indices;

        for (uint32_t i = 0; i + 2 < Indices.size(); i += 3)
        {
            const uint32_t I0 = Indices[i + 0];
            const uint32_t I1 = Indices[i + 1];
            const uint32_t I2 = Indices[i + 2];

            if (I0 >= Vertices.size() || I1 >= Vertices.size() || I2 >= Vertices.size())
            {
                continue;
            }

            Geometry::FTriangle Triangle;
            Triangle.V0 = Vertices[I0].Position;
            Triangle.V1 = Vertices[I1].Position;
            Triangle.V2 = Vertices[I2].Position;

            OutTriangles.push_back(Triangle);
        }

        return OutTriangles.size() > 0;
    }

    void UPrimitiveComponent::Update(float DeltaTime)
    {
        USceneComponent::Update(DeltaTime);

        if (bBoundsDirty)
        {
            UE_LOG(PrimitiveComponent, ELogLevel::Verbose, "Bounds dirty -> updating AABB for %s",
                   GetTypeName());
            UpdateBounds();
            bBoundsDirty = false;
        }
    }

    void UPrimitiveComponent::DescribeProperties(FComponentPropertyBuilder& Builder)
    {
        USceneComponent::DescribeProperties(Builder);
        Builder.AddColor(
            "color", L"Color", [this]() { return GetColor(); },
            [this](const FColor& InColor) { SetColor(InColor); });
    }

    void UPrimitiveComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                                ESceneShowFlags   InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_Primitives))
        {
            return;
        }

        AActor* Actor = GetOwnerActor();
        if (Actor == nullptr)
        {
            return;
        }

        FRenderCommand& MutableRenderCommand = const_cast<FRenderCommand&>(RenderCommand);
        if (MutableRenderCommand.MeshData == nullptr)
        {
            return;
        }

        if (MutableRenderCommand.Material == nullptr)
        {
            MutableRenderCommand.Material = FGeneralRenderer::GetDefaultMaterial();
        }

        MutableRenderCommand.WorldMatrix = GetRelativeMatrix();
        MutableRenderCommand.ObjectId = Actor->GetObjectId();
        MutableRenderCommand.bDrawAABB = Actor->IsSelected() || Actor->IsShowBounds();
        MutableRenderCommand.WorldAABB = GetWorldAABB();
        MutableRenderCommand.SetDefaultStates();
        MutableRenderCommand.SetStates(MutableRenderCommand.Material,
                                       MutableRenderCommand.MeshData->Topology);

        MutableRenderCommand.bIsVisible = Actor->IsVisible();
        MutableRenderCommand.bIsPickable = Actor->IsPickable();
        MutableRenderCommand.bIsSelected = Actor->IsSelected();

        OutRenderData.RenderCommands.push_back(MutableRenderCommand);
    }

    Geometry::FAABB UPrimitiveComponent::GetLocalAABB() const
    {
        if (RenderCommand.MeshData)
        {
            return Geometry::FAABB(RenderCommand.MeshData->GetMinCoord(),
                                   RenderCommand.MeshData->GetMaxCoord());
        }

        return Geometry::FAABB();
    }

    void UPrimitiveComponent::UpdateBounds()
    {
        const FMatrix               WorldMatrix = GetRelativeMatrix();
        TArray<Geometry::FTriangle> LocalTriangles;

        if (GetLocalTriangles(LocalTriangles) && !LocalTriangles.empty())
        {
            FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
            FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

            auto Expand = [&Min, &Max](const FVector& P)
            {
                Min.X = std::min(Min.X, P.X);
                Min.Y = std::min(Min.Y, P.Y);
                Min.Z = std::min(Min.Z, P.Z);

                Max.X = std::max(Max.X, P.X);
                Max.Y = std::max(Max.Y, P.Y);
                Max.Z = std::max(Max.Z, P.Z);
            };

            for (const Geometry::FTriangle& Triangle : LocalTriangles)
            {
                Expand(WorldMatrix.TransformPosition(Triangle.V0));
                Expand(WorldMatrix.TransformPosition(Triangle.V1));
                Expand(WorldMatrix.TransformPosition(Triangle.V2));
            }

            WorldAABB = Geometry::FAABB(Min, Max);
            UE_LOG(PrimitiveComponent, ELogLevel::Verbose,
                   "AABB updated from triangles for %s: min=(%.3f, %.3f, %.3f) max=(%.3f, %.3f, %.3f)",
                   GetTypeName(), WorldAABB.Min.X, WorldAABB.Min.Y, WorldAABB.Min.Z,
                   WorldAABB.Max.X, WorldAABB.Max.Y, WorldAABB.Max.Z);
            return;
        }

        WorldAABB = Geometry::TransformAABB(GetLocalAABB(), WorldMatrix);
        UE_LOG(PrimitiveComponent, ELogLevel::Verbose,
               "AABB updated from local bounds for %s: min=(%.3f, %.3f, %.3f) max=(%.3f, %.3f, %.3f)",
               GetTypeName(), WorldAABB.Min.X, WorldAABB.Min.Y, WorldAABB.Min.Z,
               WorldAABB.Max.X, WorldAABB.Max.Y, WorldAABB.Max.Z);
    }

    void UPrimitiveComponent::OnTransformChanged()
    {
        bBoundsDirty = true;
        UE_LOG(PrimitiveComponent, ELogLevel::Verbose, "Transform changed -> bounds marked dirty for %s",
               GetTypeName());
    }

} // namespace Engine::Component
