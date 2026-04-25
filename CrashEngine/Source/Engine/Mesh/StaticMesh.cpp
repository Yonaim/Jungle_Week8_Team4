// 메시 영역의 세부 동작을 구현합니다.
#include "Mesh/StaticMesh.h"
#include "Object/ObjectFactory.h"
#include "Asset/Cooked/ObjCookedData.h"
#include "Serialization/WindowsArchive.h"
#include "Engine/Profiling/MemoryStats.h"
#include "Platform/Paths.h"

IMPLEMENT_CLASS(UStaticMesh, UAsset)

static const FString EmptyPath;

namespace
{
struct FAssetPrimitiveVertex
{
    FVector Position;
    FVector Normal;
    FColor Color;
    FVector2 UV;
};

FString GetAssetStem(const FString& AssetPath)
{
    return FPaths::ToUtf8(std::filesystem::path(FPaths::ToWide(AssetPath)).stem().wstring());
}

}

FArchive& operator<<(FArchive& Ar, FStaticMeshSection& Section)
{
    Ar << Section.MaterialSlotName;
    Ar << Section.FirstIndex;
    Ar << Section.NumTriangles;
    return Ar;
}

FArchive& operator<<(FArchive& Ar, FStaticMaterial& Mat)
{
    Ar << Mat.MaterialSlotName;

    FString MaterialPath;
    if (Ar.IsSaving() && Mat.MaterialInterface)
    {
        MaterialPath = Mat.MaterialInterface->GetAssetPathFileName();
    }
    Ar << MaterialPath;

    if (Ar.IsLoading())
    {
        Mat.MaterialInterface = MaterialPath.empty()
                                    ? nullptr
                                    : FMaterialManager::Get().GetOrCreateStaticMeshMaterial(MaterialPath);
    }

    return Ar;
}

UStaticMesh::~UStaticMesh()
{
    ReleaseGPUResources();

    if (StaticMeshAsset)
    {
        const uint32 CPUSize =
            static_cast<uint32>(StaticMeshAsset->Vertices.size() * sizeof(FVertexPNCT_T)) +
            static_cast<uint32>(StaticMeshAsset->Indices.size() * sizeof(uint32));

        MemoryStats::SubStaticMeshCPUMemory(CPUSize);
        delete StaticMeshAsset;
        StaticMeshAsset = nullptr;
    }
}

void UStaticMesh::Serialize(FArchive& Ar)
{
    // 에셋이 비어있으면 로드용으로 생성
    if (Ar.IsLoading() && !StaticMeshAsset)
    {
        StaticMeshAsset = new FStaticMesh();
    }

    // 1. 지오메트리 데이터 직렬화
    StaticMeshAsset->Serialize(Ar);

    // 2. 머티리얼 데이터 직렬화 (필수!)
    Ar << StaticMaterials;

    // 3. 로딩 시 Section → MaterialIndex 매핑 캐싱 (매 프레임 문자열 비교 방지)
    if (Ar.IsLoading())
    {
        for (FStaticMeshSection& Section : StaticMeshAsset->Sections)
        {
            Section.MaterialIndex = -1;
            for (int32 i = 0; i < (int32)StaticMaterials.size(); ++i)
            {
                if (StaticMaterials[i].MaterialSlotName == Section.MaterialSlotName)
                {
                    Section.MaterialIndex = i;
                    break;
                }
            }
        }
    }
}

void UStaticMesh::InitResources(ID3D11Device* InDevice)
{
    if (!InDevice || !StaticMeshAsset)
        return;

    // CPU 메모리 추적
    const uint32 CPUSize =
        static_cast<uint32>(StaticMeshAsset->Vertices.size() * sizeof(FVertexPNCT_T)) +
        static_cast<uint32>(StaticMeshAsset->Indices.size() * sizeof(uint32));
    MemoryStats::AddStaticMeshCPUMemory(CPUSize);

    // CPU → GPU 정점 버퍼 변환
    TMeshData<FVertexPNCT_T> RenderMeshData;
    RenderMeshData.Vertices.reserve(StaticMeshAsset->Vertices.size());

    for (const FVertexPNCT_T& RawVert : StaticMeshAsset->Vertices)
    {
        FVertexPNCT_T RenderVert;
        RenderVert.Position = RawVert.Position;
        RenderVert.Normal = RawVert.Normal;
        RenderVert.Color = RawVert.Color;
        RenderVert.UV = RawVert.UV;
        RenderVert.Tangent = RawVert.Tangent;
        RenderMeshData.Vertices.push_back(RenderVert);
    }
    RenderMeshData.Indices = StaticMeshAsset->Indices;

    StaticMeshAsset->RenderBuffer = std::make_unique<FMeshBuffer>();
    StaticMeshAsset->RenderBuffer->Create(InDevice, RenderMeshData);

    // ── LOD 생성 (LOD1: 90%, LOD2: 55%, LOD3: 15%) ──
    /*if (StaticMeshAsset->Vertices.size() >= 100)
    {
        static const float LODRatios[] = { 0.9f, 0.55f, 0.15f };
        for (int lod = 0; lod < 3; ++lod)
        {
            FSimplifiedMesh Simplified = FMeshSimplifier::Simplify(
                StaticMeshAsset->Vertices, StaticMeshAsset->Indices,
                StaticMeshAsset->Sections, LODRatios[lod]);

            AdditionalLODs[lod].Sections = std::move(Simplified.Sections);

            TMeshData<FVertexPNCT_T> LODRenderData;
            LODRenderData.Vertices.reserve(Simplified.Vertices.size());
            for (const FVertexPNCT_T& RawVert : Simplified.Vertices)
            {
                FVertexPNCT_T RenderVert;
                RenderVert.Position = RawVert.Position;
                RenderVert.Normal = RawVert.Normal;
                RenderVert.Color = RawVert.Color;
                RenderVert.UV = RawVert.UV;
                LODRenderData.Vertices.push_back(RenderVert);
            }
            LODRenderData.Indices = std::move(Simplified.Indices);

            AdditionalLODs[lod].RenderBuffer = std::make_unique<FMeshBuffer>();
            AdditionalLODs[lod].RenderBuffer->Create(InDevice, LODRenderData);
        }
        bHasLOD = true;
    }*/
}

const FString& UStaticMesh::GetAssetPathFileName() const
{
    return UAsset::GetAssetPathFileName();
}

bool UStaticMesh::LoadFromCooked(const FString& AssetPath, const std::shared_ptr<Asset::FObjCookedData>& CookedMesh, ID3D11Device* InDevice)
{
    if (!CookedMesh || !CookedMesh->IsValid())
    {
        return false;
    }

    ResetAsset();

    auto NewMeshAsset = std::make_unique<FStaticMesh>();
    NewMeshAsset->PathFileName = AssetPath;

    const uint8* VertexBytes = CookedMesh->VertexData.data();
    for (uint32 VertexIndex = 0; VertexIndex < CookedMesh->VertexCount; ++VertexIndex)
    {
        const auto* SrcVertex = reinterpret_cast<const FAssetPrimitiveVertex*>(
            VertexBytes + static_cast<size_t>(VertexIndex) * CookedMesh->VertexStride);

        FVertexPNCT_T DstVertex;
        DstVertex.Position = SrcVertex->Position;
        DstVertex.Normal = SrcVertex->Normal;
        DstVertex.Color = SrcVertex->Color.ToVector4();
        DstVertex.UV = SrcVertex->UV;
        DstVertex.Tangent = FVector4(1.0f, 0.0f, 0.0f, 1.0f);
        NewMeshAsset->Vertices.push_back(DstVertex);
    }

    NewMeshAsset->Indices = CookedMesh->Indices;
    for (const Asset::FStaticMeshSectionData& SectionData : CookedMesh->Sections)
    {
        FStaticMeshSection Section;
        Section.MaterialIndex = static_cast<int32>(SectionData.MaterialIndex);
        Section.MaterialSlotName =
            (SectionData.MaterialIndex < CookedMesh->Materials.size())
                ? CookedMesh->Materials[SectionData.MaterialIndex].Name
                : "Default";
        Section.FirstIndex = SectionData.StartIndex;
        Section.NumTriangles = SectionData.IndexCount / 3u;
        NewMeshAsset->Sections.push_back(Section);
    }

    SetAssetPathFileName(AssetPath);
    SetAssetName(GetAssetStem(AssetPath));
    SetStaticMeshAsset(NewMeshAsset.release());
    SetLoaded(true);
    InitResources(InDevice);
    return true;
}

void UStaticMesh::ReleaseGPUResources()
{
    if (StaticMeshAsset && StaticMeshAsset->RenderBuffer)
    {
        StaticMeshAsset->RenderBuffer->Release();
        StaticMeshAsset->RenderBuffer.reset();
    }

    for (uint32 LOD = 0; LOD < 3; ++LOD)
    {
        if (AdditionalLODs[LOD].RenderBuffer)
        {
            AdditionalLODs[LOD].RenderBuffer->Release();
            AdditionalLODs[LOD].RenderBuffer.reset();
        }
    }
}

void UStaticMesh::ResetAsset()
{
    ReleaseGPUResources();

    if (StaticMeshAsset)
    {
        delete StaticMeshAsset;
        StaticMeshAsset = nullptr;
    }

    StaticMaterials.clear();
    bHasLOD = false;
    UAsset::ResetAsset();
}

void UStaticMesh::SetStaticMeshAsset(FStaticMesh* InMesh)
{
    StaticMeshAsset = InMesh;
    // 현재는 static mesh asset이 로드 후 고정된다고 보고, 메시 변경 dirty 갱신은 비활성화합니다.
    // MarkMeshTrianglePickingBVHDirty();

    // Section → MaterialIndex 캐싱 갱신
    if (StaticMeshAsset)
    {
        for (FStaticMeshSection& Section : StaticMeshAsset->Sections)
        {
            Section.MaterialIndex = -1;
            for (int32 i = 0; i < (int32)StaticMaterials.size(); ++i)
            {
                if (StaticMaterials[i].MaterialSlotName == Section.MaterialSlotName)
                {
                    Section.MaterialIndex = i;
                    break;
                }
            }
        }
        EnsureMeshTrianglePickingBVHBuilt();
    }
}

FStaticMesh* UStaticMesh::GetStaticMeshAsset() const
{
    return StaticMeshAsset;
}

void UStaticMesh::SetStaticMaterials(TArray<FStaticMaterial>&& InMaterials)
{
    StaticMaterials = InMaterials;
}

const TArray<FStaticMaterial>& UStaticMesh::GetStaticMaterials() const
{
    return StaticMaterials;
}

void UStaticMesh::EnsureMeshTrianglePickingBVHBuilt() const
{
    if (!StaticMeshAsset)
    {
        return;
    }

    MeshTrianglePickingBVH.EnsureBuilt(*StaticMeshAsset);
}

bool UStaticMesh::RaycastMeshTrianglesWithBVHLocal(const FVector& LocalOrigin, const FVector& LocalDirection, FHitResult& OutHitResult) const
{
    if (!StaticMeshAsset)
    {
        return false;
    }

    EnsureMeshTrianglePickingBVHBuilt();
    return MeshTrianglePickingBVH.RaycastLocal(LocalOrigin, LocalDirection, *StaticMeshAsset, OutHitResult);
}

FMeshBuffer* UStaticMesh::GetLODMeshBuffer(uint32 LODLevel) const
{
    if (LODLevel == 0 && StaticMeshAsset)
        return StaticMeshAsset->RenderBuffer.get();
    if (LODLevel >= 1 && LODLevel <= 3 && bHasLOD)
        return AdditionalLODs[LODLevel - 1].RenderBuffer.get();
    return StaticMeshAsset ? StaticMeshAsset->RenderBuffer.get() : nullptr;
}

static const TArray<FStaticMeshSection> EmptySections;

void FStaticMesh::Serialize(FArchive& Ar)
{
    Ar << PathFileName;
    Ar << Vertices;
    Ar << Indices;
    Ar << Sections;
}

const TArray<FStaticMeshSection>& UStaticMesh::GetLODSections(uint32 LODLevel) const
{
    if (LODLevel == 0 && StaticMeshAsset)
        return StaticMeshAsset->Sections;
    if (LODLevel >= 1 && LODLevel <= 3 && bHasLOD)
        return AdditionalLODs[LODLevel - 1].Sections;
    return StaticMeshAsset ? StaticMeshAsset->Sections : EmptySections;
}
