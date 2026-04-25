#pragma once
#include "StaticMeshActor.h"


class ENGINE_API AConeActor : public AStaticMeshActor
{
    DECLARE_RTTI(AConeActor, AStaticMeshActor)
    
public:
    AConeActor();
    ~AConeActor() override = default;

    EBasicMeshType GetMeshType() const override { return EBasicMeshType::Cone; }
};
