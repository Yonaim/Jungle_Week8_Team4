#pragma once

#include "StaticMeshActor.h"

class ENGINE_API ATriangleActor : public AStaticMeshActor
{
    DECLARE_RTTI(ATriangleActor, AStaticMeshActor)

  public:
    ATriangleActor();
    ~ATriangleActor() override = default;

    EBasicMeshType GetMeshType() const override { return EBasicMeshType::Triangle; };

};
