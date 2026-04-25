#include "UUIDComponent.h"

#include "Core/Misc/BitMaskEnum.h"
#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Engine/Game/Actor.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/SceneView.h"

namespace Engine::Component
{
    UUUIDComponent::UUUIDComponent()
    {
        //SetFontPath("Font\\JetBrainsMono\\JetBrainsMono_Medium.Font");
        SetColor(FColor::White());
        SetBillboard(true);
    }

    void UUUIDComponent::RefreshFromOwner()
    {
        if (AActor* OwnerActor = GetOwnerActor())
        {
            SetText("UUID: " + std::to_string(OwnerActor->UUID));
        }
    }

    FMatrix UUUIDComponent::GetRenderPlacementWorld(const AActor& InOwnerActor) const
    {
        return FMatrix::MakeTranslation(ComputeWorldAnchor(InOwnerActor));
    }

    FVector UUUIDComponent::GetRenderPlacementOffset(const AActor& InOwnerActor) const
    {
        (void)InOwnerActor;
        return FVector::ZeroVector;
    }

    void UUUIDComponent::CollectRenderData(FSceneRenderData& OutRenderData,
                                           ESceneShowFlags   InShowFlags) const
    {
        if (!IsFlagSet(InShowFlags, ESceneShowFlags::SF_UUIDText))
            return;
        
        FRenderCommand Command;
        // TextRenderComponent::CreateRenderCommand()를 그대로 사용
        if (CreateRenderCommand(OutRenderData, InShowFlags, Command)) 
            return;
        
        // 부모(액터)의 Scale에 영향 받지 않도록 World Matrix 덮어쓰기
        AActor* Actor = GetOwnerActor();
        const FMatrix PlacementWorld = GetRenderPlacementWorld(*Actor);
        const FVector PlacementOffset = GetRenderPlacementOffset(*Actor);
        FVector       Origin = PlacementWorld.GetOrigin() + PlacementOffset + BillboardOffset;
        
        const FMatrix CameraWorld = OutRenderData.SceneView->GetViewMatrix().GetInverse();
        FVector       RightAxis = CameraWorld.GetRightVector();
        FVector       UpAxis = CameraWorld.GetUpVector();
        FVector       ForwardAxis = CameraWorld.GetForwardVector();

        FVector       Row0 = UpAxis;
        FVector       Row1 = RightAxis;
        FVector       Row2 = -ForwardAxis;

        Command.WorldMatrix.M[0][0] = Row0.X; Command.WorldMatrix.M[0][1] = Row0.Y; Command.WorldMatrix.M[0][2] = Row0.Z; Command.WorldMatrix.M[0][3] = 0.0f;
        Command.WorldMatrix.M[1][0] = Row1.X; Command.WorldMatrix.M[1][1] = Row1.Y; Command.WorldMatrix.M[1][2] = Row1.Z; Command.WorldMatrix.M[1][3] = 0.0f;
        Command.WorldMatrix.M[2][0] = Row2.X; Command.WorldMatrix.M[2][1] = Row2.Y; Command.WorldMatrix.M[2][2] = Row2.Z; Command.WorldMatrix.M[2][3] = 0.0f;
        Command.WorldMatrix.M[3][0] = Origin.X; Command.WorldMatrix.M[3][1] = Origin.Y; Command.WorldMatrix.M[3][2] = Origin.Z; Command.WorldMatrix.M[3][3] = 1.0f;
        
        // Wireframe 옵션 무시하게 설정
        Command.bIgnoreWireFrame = true;
        OutRenderData.RenderCommands.push_back(Command);
    }

    FVector UUUIDComponent::ComputeWorldAnchor(const AActor& InOwnerActor) const
    {
        const auto* PrimitiveRoot =
            dynamic_cast<const UPrimitiveComponent*>(InOwnerActor.GetRootComponent());
        if (PrimitiveRoot != nullptr)
        {
            const Geometry::FAABB& WorldAABB = PrimitiveRoot->GetWorldAABB();
            FVector Anchor = (WorldAABB.Min + WorldAABB.Max) * 0.5f;
            Anchor.Z = WorldAABB.Max.Z + DefaultVerticalPadding;
            return Anchor;
        }

        FVector Anchor = InOwnerActor.GetWorldMatrix().GetOrigin();
        Anchor.Z += DefaultVerticalPadding;
        return Anchor;
    }

    REGISTER_CLASS(Engine::Component, UUUIDComponent)
} // namespace Engine::Component
