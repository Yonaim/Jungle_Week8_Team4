#pragma once
#include "Actor.h"

namespace Engine::Component
{
    class UPrimitiveComponent;
    class USubUVAnimatedComponent;
} // namespace Engine::Component

class ENGINE_API AFlipbookActor : public AActor
{
    DECLARE_RTTI(AFlipbookActor, AActor)
  public:
    AFlipbookActor();
    ~AFlipbookActor() override = default;

    Engine::Component::USubUVAnimatedComponent* GetSubUVAnimatedComponent() const;
};