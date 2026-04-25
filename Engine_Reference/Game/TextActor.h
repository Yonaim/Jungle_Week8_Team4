#pragma once

#include "Engine/Game/Actor.h"

namespace Engine::Component
{
    class UPrimitiveComponent;
    class UTextRenderComponent;
}

class ENGINE_API ATextActor : public AActor
{
    DECLARE_RTTI(ATextActor, AActor)

  public:
    ATextActor();
    ~ATextActor() override = default;

    Engine::Component::UTextRenderComponent* GetTextComponent() const;

    bool           IsRenderable() const override;
    bool           IsSelected() const override;
    FColor         GetColor() const override;
    EBasicMeshType GetMeshType() const override;
    uint32         GetObjectId() const override;

  private:
    Engine::Component::UPrimitiveComponent* GetPrimitiveComponent() const;
};
