#pragma once
#include "StaticMeshActor.h"

class ENGINE_API ARingActor : public AStaticMeshActor
{
    DECLARE_RTTI(ARingActor, AStaticMeshActor)
    
public:
    ARingActor();
    ~ARingActor() override = default;
    
    EBasicMeshType GetMeshType() const override { return EBasicMeshType::Ring; }
};
