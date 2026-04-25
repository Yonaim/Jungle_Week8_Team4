#include "SpriteActor.h"

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Engine/Component/Sprite/PaperSpriteComponent.h"

ASpriteActor::ASpriteActor()
{
    auto* SpriteComponent = new Engine::Component::UPaperSpriteComponent();
    SpriteComponent->SetColor({0.8f, 0.8f, 0.8f, 1.f});
    AddOwnedComponent(SpriteComponent, true);

    Name = "SpriteActor";
}

Engine::Component::UPaperSpriteComponent* ASpriteActor::GetSpriteComponent() const
{
    return Cast<Engine::Component::UPaperSpriteComponent>(RootComponent);
}

bool ASpriteActor::IsRenderable() const { return GetPrimitiveComponent() != nullptr; }

bool ASpriteActor::IsSelected() const
{
    if (RootComponent == nullptr)
    {
        return false;
    }

    return RootComponent->IsSelected();
}

FColor ASpriteActor::GetColor() const
{
    if (const auto* PrimitiveComponent = GetPrimitiveComponent())
    {
        return PrimitiveComponent->GetColor();
    }

    return FColor::White();
}

EBasicMeshType ASpriteActor::GetMeshType() const
{
    if (const auto* SpriteComponent = GetSpriteComponent())
    {
        return SpriteComponent->GetBasicMeshType();
    }

    return AActor::GetMeshType();
}

uint32 ASpriteActor::GetObjectId() const { return UUID; }

Engine::Component::UPrimitiveComponent* ASpriteActor::GetPrimitiveComponent() const
{
    return Cast<Engine::Component::UPrimitiveComponent>(RootComponent);
}

REGISTER_CLASS(, ASpriteActor)
