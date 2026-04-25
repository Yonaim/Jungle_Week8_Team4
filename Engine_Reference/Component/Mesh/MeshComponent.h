#pragma once
#include "Engine/Component/Core/PrimitiveComponent.h"

/*
    UMeshComponent(계층 구조용 뼈대)
*/
namespace Engine::Component
{
    
    class ENGINE_API UMeshComponent : public UPrimitiveComponent
    {
        DECLARE_RTTI(UMeshComponent, UPrimitiveComponent)
      public:
        UMeshComponent() = default;
        ~UMeshComponent() override = default;
    };
};
