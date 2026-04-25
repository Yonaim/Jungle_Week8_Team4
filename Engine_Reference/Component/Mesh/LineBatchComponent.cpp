#include "LineBatchComponent.h"
#include <algorithm>

#include "Core/Misc/BitMaskEnum.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/D3D11/GeneralRenderer.h"
#include "Renderer/Primitive/FMeshData.h"

namespace Engine::Component
{
    ULineBatchComponent::ULineBatchComponent()
    {
        UE_LOG(ULineBatchComponent, ELogLevel::Verbose, "Test");
    }

    Geometry::FAABB ULineBatchComponent::GetLocalAABB() const
    {
        // 디버그용 선들이므로 컬링 등에서 제외되지 않도록 주의해야 함.
        // 일단 빈 박스를 반환합니다.
        return Geometry::FAABB();
    }

    void ULineBatchComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                                ESceneShowFlags   InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_Primitives))
            return;

        if (!BatchMeshData)
        {
            BatchMeshData = std::make_shared<FMeshData>();
            BatchMeshData->bIsDynamicMesh = true;
            BatchMeshData->Topology = EMeshTopology::EMT_LineList;
        }

        BatchMeshData->Vertices.clear();
        BatchMeshData->Indices.clear();

        for (const auto& Line : Lines)
        {
            uint32 StartIdx = static_cast<uint32>(BatchMeshData->Vertices.size());

            FPrimitiveVertex V0;
            V0.Position = Line.Start;
            V0.Color = Line.Color;
            V0.Normal = FVector::ZeroVector;
            V0.UV = FVector2::ZeroVector;

            FPrimitiveVertex V1;
            V1.Position = Line.End;
            V1.Color = Line.Color;
            V1.Normal = FVector::ZeroVector;
            V1.UV = FVector2::ZeroVector;

            BatchMeshData->Vertices.push_back(V0);
            BatchMeshData->Vertices.push_back(V1);

            BatchMeshData->Indices.push_back(StartIdx);
            BatchMeshData->Indices.push_back(StartIdx + 1);
        }
            
        FRenderCommand Cmd;
        Cmd.MeshData = BatchMeshData.get();
        Cmd.Material = FGeneralRenderer::GetDefaultMaterial(); // Fallback to default or line material
        Cmd.WorldMatrix = FMatrix::Identity;
        Cmd.RenderLayer = ERenderLayer::Default;
        Cmd.SetDefaultStates();
        Cmd.RasterizerOption.FillMode = D3D11_FILL_WIREFRAME;
        Cmd.SetStates(Cmd.Material, EMeshTopology::EMT_LineList);

        OutRenderData.RenderCommands.push_back(Cmd);
    }

    void ULineBatchComponent::Update(float InDeltaTime)
    {
        UPrimitiveComponent::Update(InDeltaTime);

        // 선들의 수명 관리
        auto It = std::remove_if(Lines.begin(), Lines.end(), [InDeltaTime](FLineData& Line) {
            if (Line.RemainingLifeTime > 0.0f)
            {
                Line.RemainingLifeTime -= InDeltaTime;
                return Line.RemainingLifeTime <= 0.0f;
            }
            // 음수(-1.0f 등)인 경우 영구 유지
            if (Line.RemainingLifeTime < 0.0f)
            {
                return false;
            }
            // 0.0f인 경우 한 프레임만 출력 후 제거
            return true;
        });

        if (It != Lines.end())
        {
            Lines.erase(It, Lines.end());
        }
    }

    void ULineBatchComponent::AddLine(const FVector& InStart, const FVector& InEnd, const FColor& InColor, float InLifeTime)
    {
        Lines.push_back({InStart, InEnd, InColor, InLifeTime});
    }

    void ULineBatchComponent::AddBox(const FVector& InMin, const FVector& InMax, const FColor& InColor, float InLifeTime)
    {
        const FVector P000(InMin.X, InMin.Y, InMin.Z);
        const FVector P100(InMax.X, InMin.Y, InMin.Z);
        const FVector P010(InMin.X, InMax.Y, InMin.Z);
        const FVector P110(InMax.X, InMax.Y, InMin.Z);
        const FVector P001(InMin.X, InMin.Y, InMax.Z);
        const FVector P101(InMax.X, InMin.Y, InMax.Z);
        const FVector P011(InMin.X, InMax.Y, InMax.Z);
        const FVector P111(InMax.X, InMax.Y, InMax.Z);

        AddLine(P000, P100, InColor, InLifeTime);
        AddLine(P100, P110, InColor, InLifeTime);
        AddLine(P110, P010, InColor, InLifeTime);
        AddLine(P010, P000, InColor, InLifeTime);

        AddLine(P001, P101, InColor, InLifeTime);
        AddLine(P101, P111, InColor, InLifeTime);
        AddLine(P111, P011, InColor, InLifeTime);
        AddLine(P011, P001, InColor, InLifeTime);

        AddLine(P000, P001, InColor, InLifeTime);
        AddLine(P100, P101, InColor, InLifeTime);
        AddLine(P110, P111, InColor, InLifeTime);
        AddLine(P010, P011, InColor, InLifeTime);
    }

    void ULineBatchComponent::AddSphere(const FVector& InCenter, float InRadius, int32 InSegments, const FColor& InColor, float InLifeTime)
    {
        if (InSegments < 3) InSegments = 16;
        const float Step = 2.0f * 3.1415926535f / static_cast<float>(InSegments);

        for (int32 i = 0; i < InSegments; ++i)
        {
            const float Angle = static_cast<float>(i) * Step;
            const float NextAngle = static_cast<float>(i + 1) * Step;

            const float CosA = std::cos(Angle);
            const float SinA = std::sin(Angle);
            const float CosNext = std::cos(NextAngle);
            const float SinNext = std::sin(NextAngle);

            // XY 루프
            AddLine(InCenter + FVector(CosA * InRadius, SinA * InRadius, 0.0f),
                    InCenter + FVector(CosNext * InRadius, SinNext * InRadius, 0.0f), InColor, InLifeTime);
            // YZ 루프
            AddLine(InCenter + FVector(0.0f, CosA * InRadius, SinA * InRadius),
                    InCenter + FVector(0.0f, CosNext * InRadius, SinNext * InRadius), InColor, InLifeTime);
            // ZX 루프
            AddLine(InCenter + FVector(SinA * InRadius, 0.0f, CosA * InRadius),
                    InCenter + FVector(SinNext * InRadius, 0.0f, CosNext * InRadius), InColor, InLifeTime);
        }
    }

    void ULineBatchComponent::ClearLines()
    {
        Lines.clear(); 
    }

    REGISTER_CLASS(Engine::Component, ULineBatchComponent)
} // namespace Engine::Component
