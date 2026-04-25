#include "StaticMesh.h"

#include "Engine/Asset/Material.h"
#include "Core/Logging/LogMacros.h"
#include "Core/Misc/Paths.h"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstring>
#include <filesystem>

REGISTER_CLASS(, UStaticMesh)

namespace
{
    static FString BuildAssetNameFromPath(const FString& InAssetPath)
    {
        if (InAssetPath.empty())
        {
            return {};
        }

        const std::filesystem::path FilePath = FPaths::PathFromUtf8(InAssetPath);
        return FPaths::Utf8FromPath(FilePath.stem());
    }
} // namespace

bool UStaticMesh::LoadFromCooked(const FString&                  InAssetPath,
                                 std::shared_ptr<FObjCookedData> InCookedData,
                                 RHI::FDynamicRHI&               InDynamicRHI)
{
    if (InCookedData == nullptr)
    {
        UE_LOG(StaticMesh, ELogLevel::Error, "Static mesh asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    if (CookedData == InCookedData && GetAssetPath() == InAssetPath && RenderResource != nullptr)
    {
        SetLoaded(true);
        return true;
    }

    std::shared_ptr<FStaticMeshRenderResource> NewRenderResource =
        FStaticMeshRenderResource::Create(*InCookedData, InDynamicRHI);
    if (NewRenderResource == nullptr)
    {
        UE_LOG(StaticMesh, ELogLevel::Error, "Static mesh asset load failed: %s", InAssetPath.c_str());
        return false;
    }

    SetAssetPath(InAssetPath);
    SetAssetName(BuildAssetNameFromPath(InAssetPath));
    SetCookedData(std::move(InCookedData));
    SetRenderResource(std::move(NewRenderResource));

    Sections.clear();
    MaterialSlots.clear();

    if (CookedData)
    {
        MaterialSlots.resize(CookedData->Materials.size(), nullptr);
        Sections.reserve(CookedData->Sections.size());

        for (const FStaticMeshSectionData& CookedSection : CookedData->Sections)
        {
            FStaticMeshSection Section;
            Section.FirstIndex = CookedSection.StartIndex;
            Section.IndexCount = CookedSection.IndexCount;
            Section.MaterialIndex = CookedSection.MaterialIndex;
            Sections.push_back(Section);
        }
    }

    Build();
    SetLoaded(true);
    return true;
}

FString UStaticMesh::GetMeshName() const { return GetAssetName(); }

const TArray<uint8>& UStaticMesh::GetVerticesData() const
{
    assert(CookedData != nullptr);
    return CookedData->VertexData;
}

const TArray<uint32>& UStaticMesh::GetIndicesData() const
{
    assert(CookedData != nullptr);
    return CookedData->Indices;
}

uint32 UStaticMesh::GetVertexStride() const { return CookedData ? CookedData->VertexStride : 0; }

uint32 UStaticMesh::GetVerticesCount() const { return CookedData ? CookedData->VertexCount : 0; }

uint32 UStaticMesh::GetIndicesCount() const
{
    return CookedData ? static_cast<uint32>(CookedData->Indices.size()) : 0;
}

EStaticMeshVertexFormat UStaticMesh::GetVertexFormat() const
{
    return CookedData ? CookedData->VertexFormat : EStaticMeshVertexFormat::P;
}

void UStaticMesh::Build() { CalculateAABB(); }

bool UStaticMesh::IsValidLowLevel() const
{
    if (CookedData == nullptr)
    {
        return false;
    }

    if (CookedData->VertexData.empty() || CookedData->Indices.empty())
    {
        return false;
    }

    if (CookedData->VertexStride == 0 || CookedData->VertexCount == 0)
    {
        return false;
    }

    if (CookedData->VertexData.size() <
        static_cast<size_t>(CookedData->VertexStride) * CookedData->VertexCount)
    {
        return false;
    }

    return true;
}

void UStaticMesh::CalculateAABB()
{
    if (!CookedData || CookedData->VertexData.empty() ||
        CookedData->VertexStride < sizeof(FVector) || CookedData->VertexCount == 0)
    {
        CachedAABB = Geometry::FAABB();
        return;
    }

    FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
    FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    const uint8* VertexBase = CookedData->VertexData.data();
    const uint32 VertexStride = CookedData->VertexStride;
    const uint32 VertexCount = CookedData->VertexCount;

    for (uint32 i = 0; i < VertexCount; ++i)
    {
        const uint8* VertexPtr = VertexBase + static_cast<size_t>(i) * VertexStride;

        FVector Position;
        std::memcpy(&Position, VertexPtr, sizeof(FVector));

        Min.X = std::min(Min.X, Position.X);
        Min.Y = std::min(Min.Y, Position.Y);
        Min.Z = std::min(Min.Z, Position.Z);

        Max.X = std::max(Max.X, Position.X);
        Max.Y = std::max(Max.Y, Position.Y);
        Max.Z = std::max(Max.Z, Position.Z);
    }

    CachedAABB.Min = Min;
    CachedAABB.Max = Max;

    UE_LOG(StaticMesh, ELogLevel::Verbose,
           "Static mesh AABB updated: %s min=(%.3f, %.3f, %.3f) max=(%.3f, %.3f, %.3f)",
           GetAssetPath().c_str(), CachedAABB.Min.X, CachedAABB.Min.Y, CachedAABB.Min.Z,
           CachedAABB.Max.X, CachedAABB.Max.Y, CachedAABB.Max.Z);
}
