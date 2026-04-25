#pragma once

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Renderer/Types/BasicMeshType.h"

namespace Engine::Component
{
    /** 선 하나에 대한 데이터 */
    struct FLineData
    {
        FVector Start;
        FVector End;
        FColor  Color;
        float   RemainingLifeTime;
    };
    
    class ENGINE_API ULineBatchComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(ULineBatchComponent, UPrimitiveComponent)

    public:
        ULineBatchComponent();
        ~ULineBatchComponent() override = default;

        // UPrimitiveComponent 인터페이스 구현
        EBasicMeshType GetBasicMeshType() const override { return EBasicMeshType::None; }

        void CollectRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const override;
        
    protected:
        Geometry::FAABB GetLocalAABB() const override;

    public:
        /** 매 프레임 선들의 수명 업데이트. */
        void Update(float InDeltaTime) override;

        void AddLine(const FVector& InStart, const FVector& InEnd, const FColor& InColor, float InLifeTime = 0.0f);

        void AddBox(const FVector& InMin, const FVector& InMax, const FColor& InColor, float InLifeTime = 0.0f);

        void AddSphere(const FVector& InCenter, float InRadius, int32 InSegments, const FColor& InColor, float InLifeTime = 0.0f);

        const TArray<FLineData>& GetLines() const { return Lines; }

        void ClearLines();

    private:
        TArray<FLineData> Lines;
        mutable std::shared_ptr<FMeshData> BatchMeshData;
    };
} // namespace Engine::Component
