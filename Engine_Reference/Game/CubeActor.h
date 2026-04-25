#pragma once

#include "Engine/Game/StaticMeshActor.h"

class ENGINE_API ACubeActor : public AStaticMeshActor
{
    DECLARE_RTTI(ACubeActor, AStaticMeshActor)

  public:
    ACubeActor();
    ~ACubeActor() override = default;

    EBasicMeshType GetMeshType() const override { return EBasicMeshType::Cube; }
};
