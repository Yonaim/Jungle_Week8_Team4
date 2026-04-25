#pragma once

#include "StaticMeshActor.h"

class ENGINE_API AQuadActor : public AStaticMeshActor
{
    DECLARE_RTTI(AQuadActor, AStaticMeshActor)

  public:
    AQuadActor();
    ~AQuadActor() override = default;

    EBasicMeshType GetMeshType() const override { return EBasicMeshType::Quad; };
};
