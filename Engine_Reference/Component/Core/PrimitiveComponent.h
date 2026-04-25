#pragma once

#include "Core/Geometry/Primitives/AABB.h"
#include "Core/Math/Color.h"
#include "SceneComponent.h"
#include "Renderer/Types/SceneShowFlags.h"
#include "Renderer/RenderCommand.h"

enum class EBasicMeshType : uint8;
struct FSceneRenderData;

namespace Engine::Component
{
    class ENGINE_API UPrimitiveComponent : public Engine::Component::USceneComponent
    {
        DECLARE_RTTI(UPrimitiveComponent, USceneComponent)

    public:
        UPrimitiveComponent() = default;
        ~UPrimitiveComponent() override = default;

        virtual EBasicMeshType GetBasicMeshType() const = 0;

        const FColor& GetColor() const;
        void          SetColor(const FColor& NewColor);

        const Geometry::FAABB& GetWorldAABB() const;
        bool                   GetWorldAABB(Geometry::FAABB& OutWorldAABB) const;

        virtual bool GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const;

        void Update(float DeltaTime) override;
        void DescribeProperties(FComponentPropertyBuilder& Builder) override;

        bool IsShowBounds() const override { return bShowAABB; }
        void SetShowBounds(bool bNewShowBounds) override { bShowAABB = bNewShowBounds; }

        virtual void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const;

    protected:
        virtual Geometry::FAABB GetLocalAABB() const;
        virtual void            UpdateBounds();
        void                    OnTransformChanged() override;

    protected:
        FColor          Color = FColor::White();
        Geometry::FAABB WorldAABB;
        bool            bShowAABB = false;
        bool            bBoundsDirty = true;
        FRenderCommand  RenderCommand;
    };
} // namespace Engine::Component