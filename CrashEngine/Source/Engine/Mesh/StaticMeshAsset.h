// 메시 영역에서 공유되는 타입과 인터페이스를 정의합니다.
#pragma once

#include "Core/CoreTypes.h"
#include "Math/Vector.h"
#include "Engine/Object/Object.h"
#include "Render/RHI/D3D11/Buffers/Buffers.h"
#include "Serialization/Archive.h"
#include "Engine/Object/FName.h"
#include "Materials/Material.h"
#include "Materials/MaterialManager.h"
#include "Render/RHI/D3D11/Buffers/VertexTypes.h"
#include <memory>
#include <algorithm>

// FStaticMeshSection는 메시 데이터와 렌더 제출 정보를 다룹니다.
struct FStaticMeshSection
{
    int32 MaterialIndex = -1; // Index into UStaticMesh's FStaticMaterial array. Cached to avoid per-frame string comparison.
    FString MaterialSlotName;
    uint32 FirstIndex;
    uint32 NumTriangles;

    friend FArchive& operator<<(FArchive& Ar, FStaticMeshSection& Section);
};

// FStaticMaterial는 머티리얼 파라미터와 렌더 리소스를 다룹니다.
struct FStaticMaterial
{
    // std::shared_ptr<class UMaterialInterface> MaterialInterface;
    UMaterial* MaterialInterface;
    FString MaterialSlotName = "None"; // "None"은 특별한 슬롯 이름으로, OBJ 파일에서 머티리얼이 지정되지 않은 섹션에 할당됩니다.

    friend FArchive& operator<<(FArchive& Ar, FStaticMaterial& Mat);
};

// Cooked Data — GPU용 정점/인덱스
// FStaticMeshLODResources in UE5
struct FStaticMesh
{
    FString PathFileName;
    TArray<FVertexPNCT_T> Vertices;
    TArray<uint32> Indices;

    TArray<FStaticMeshSection> Sections;

    std::unique_ptr<FMeshBuffer> RenderBuffer;

    // 메시 로컬 바운드 캐시 (정점 순회 1회로 계산)
    FVector BoundsCenter = FVector(0, 0, 0);
    FVector BoundsExtent = FVector(0, 0, 0);
    bool bBoundsValid = false;

    void CacheBounds()
    {
        bBoundsValid = false;
        if (Vertices.empty())
            return;

        FVector LocalMin = Vertices[0].Position;
        FVector LocalMax = Vertices[0].Position;
        for (const FVertexPNCT_T& V : Vertices)
        {
            LocalMin.X = (std::min)(LocalMin.X, V.Position.X);
            LocalMin.Y = (std::min)(LocalMin.Y, V.Position.Y);
            LocalMin.Z = (std::min)(LocalMin.Z, V.Position.Z);
            LocalMax.X = (std::max)(LocalMax.X, V.Position.X);
            LocalMax.Y = (std::max)(LocalMax.Y, V.Position.Y);
            LocalMax.Z = (std::max)(LocalMax.Z, V.Position.Z);
        }

        BoundsCenter = (LocalMin + LocalMax) * 0.5f;
        BoundsExtent = (LocalMax - LocalMin) * 0.5f;
        bBoundsValid = true;
    }

    void Serialize(FArchive& Ar);
};
