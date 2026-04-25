#pragma once

#include <cfloat>
#include <cstring>
#include <filesystem>
#include <memory>
#include <utility>

#include "Engine/Asset/Asset.h"
#include "Asset/Cooked/ObjCookedData.h"
#include "Asset/Runtime/StaticMeshRenderResource.h"
#include "Core/Geometry/Primitives/AABB.h"
#include "RHI/DynamicRHI.h"

class UMaterial;

using namespace Asset;

struct FStaticMeshSection
{
    uint32 FirstIndex = 0;
    uint32 IndexCount = 0;
    uint32 MaterialIndex = 0;
};

class ENGINE_API UStaticMesh : public UAsset
{
    DECLARE_RTTI(UStaticMesh, UAsset)

  public:
    const std::shared_ptr<FObjCookedData>& GetCookedData() const { return CookedData; }

    void SetCookedData(std::shared_ptr<FObjCookedData> InCookedData)
    {
        CookedData = std::move(InCookedData);
    }

    const std::shared_ptr<FStaticMeshRenderResource>& GetRenderResource() const
    {
        return RenderResource;
    }

    void SetRenderResource(std::shared_ptr<FStaticMeshRenderResource> InRenderResource)
    {
        RenderResource = std::move(InRenderResource);
    }

    void ResetRenderResource()
    {
        if (RenderResource)
        {
            RenderResource->Reset();
        }
        RenderResource.reset();
    }

    bool LoadFromCooked(const FString&                         InAssetPath,
                        std::shared_ptr<FObjCookedData> InCookedData,
                        RHI::FDynamicRHI&                      InDynamicRHI);

    FString GetMeshName() const;

    const TArray<uint8>&  GetVerticesData() const;
    const TArray<uint32>& GetIndicesData() const;

    uint32 GetVertexStride() const;
    uint32 GetVerticesCount() const;
    uint32 GetIndicesCount() const;

    const TArray<FStaticMeshSection>& GetSections() const { return Sections; }
    const TArray<UMaterial*>&         GetMaterialSlots() const { return MaterialSlots; }
    TArray<UMaterial*>&               GetMaterialSlots() { return MaterialSlots; }

    EStaticMeshVertexFormat GetVertexFormat() const;

    void Build();
    bool IsValidLowLevel() const;

    const Geometry::FAABB& GetAABB() const { return CachedAABB; }
    void                   CalculateAABB();

  private:
    std::shared_ptr<FObjCookedData>     CookedData;
    std::shared_ptr<FStaticMeshRenderResource> RenderResource;
    Geometry::FAABB                            CachedAABB;

  public:
    TArray<FStaticMeshSection> Sections;
    TArray<UMaterial*>         MaterialSlots;
};
