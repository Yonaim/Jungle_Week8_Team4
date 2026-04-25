#pragma once

#include "MeshActor.h"

namespace Engine::Component
{
    class UStaticMeshComponent;
    class UPrimitiveComponent;
} // namespace Engine::Component

class ENGINE_API AStaticMeshActor : public AMeshActor
{
    DECLARE_RTTI(AStaticMeshActor, AMeshActor)
  public:
    AStaticMeshActor();
    ~AStaticMeshActor() override = default;

    Engine::Component::UStaticMeshComponent* GetStaticMeshComponent() const;

  protected:
    void SetDefaultStaticMeshPath(const FString& InPath);

    bool           IsRenderable() const override;
    bool           IsSelected() const override;
    FColor         GetColor() const override;
    EBasicMeshType GetMeshType() const override;
    uint32         GetObjectId() const override;

  private:
    Engine::Component::UPrimitiveComponent* GetPrimitiveComponent() const;
};
