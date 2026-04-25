#include "EffectActor.h"

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Engine/Component/Sprite/SubUVAnimatedComponent.h"

AEffectActor::AEffectActor()
{
    auto* EffectComponent = new Engine::Component::USubUVAnimatedComponent();
    EffectComponent->SetColor({0.8f, 0.8f, 0.8f, 1.f});
    AddOwnedComponent(EffectComponent, true);

    Name = "EffectActor";
}

Engine::Component::USubUVAnimatedComponent* AEffectActor::GetEffectComponent() const
{
    return Cast<Engine::Component::USubUVAnimatedComponent>(RootComponent);
}

REGISTER_CLASS(, AEffectActor)
