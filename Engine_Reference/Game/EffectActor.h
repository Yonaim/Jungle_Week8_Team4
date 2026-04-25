#pragma once

#include "Actor.h"

namespace Engine::Component
{
    class UPrimitiveComponent;
    class USubUVAnimatedComponent;
}

class ENGINE_API AEffectActor : public AActor
{
    DECLARE_RTTI(AEffectActor, AActor)

  public:
    AEffectActor();
    ~AEffectActor() override = default;

    Engine::Component::USubUVAnimatedComponent* GetEffectComponent() const;
};
