#pragma once
#include "MeshComponent.h"
#include "Renderer/Types/BasicMeshType.h"
#include "Engine/Asset/Material.h"

class UStaticMesh;

struct FMeshData;

namespace Engine::Component
{
    class ENGINE_API UStaticMeshComponent : public UMeshComponent
    {
        DECLARE_RTTI(UStaticMeshComponent, UMeshComponent)
      public:
        UStaticMeshComponent() = default;
        ~UStaticMeshComponent() override = default;

        const FString& GetStaticMeshPath() const;
        void           SetStaticMeshPath(const FString& InPath);

        const UStaticMesh* GetStaticMeshAsset() const { return StaticMesh; }
        UStaticMesh*       GetStaticMeshAsset() { return StaticMesh; }
        void               SetStaticMeshAsset(UStaticMesh* InStaticMesh);

        void SetEnableUVScroll(bool bInEnableUVScroll) { bEnableUVScroll = bInEnableUVScroll; }
        bool GetEnableUVScroll() const { return bEnableUVScroll; }

        void SetUVScrollSpeed(float InScrollSpeed) { ScrollSpeed = InScrollSpeed; }
        float GetUVScrollSpeed() const { return ScrollSpeed; }

        // Primitive
        virtual void CollectRenderData(FSceneRenderData& OutRenderData,
                                       ESceneShowFlags   InShowFlags) const override;
        void         Update(float DeltaTime) override;

        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::None; }

        void DescribeProperties(FComponentPropertyBuilder& Builder) override;
        bool GetLocalTriangles(TArray<Geometry::FTriangle>& OutTriangles) const override;

        int32      GetMaterialSlotCount() const;
        bool       IsValidMaterialSlotIndex(int32 SlotIndex) const;
        FString    GetMaterialSlotName(int32 SlotIndex) const;

        UMaterial* GetMaterial(int32 SlotIndex) const;
        UMaterial* GetOverrideMaterial(int32 SlotIndex) const;
        bool       HasMaterialOverride(int32 SlotIndex) const;
        void       SetMaterial(int32 SlotIndex, UMaterial* InMaterial);
        void       ClearMaterialOverride(int32 SlotIndex);
        void       ClearAllMaterialOverrides();

      protected:
        Geometry::FAABB GetLocalAABB() const override;

      private:
        void SyncMaterialOverridesWithStaticMesh();

      private:
        FString MeshPath;

      protected:
        UStaticMesh*       StaticMesh = nullptr;
        TArray<UMaterial*> OverrideMaterials;

        mutable std::shared_ptr<FMeshData> MeshData;

        //UV Scroll
        float ScrollSpeed = 1.0f;
        FVector2 UVOffset = FVector2::Zero();
        bool     bEnableUVScroll = false;
    };

} // namespace Engine::Component
