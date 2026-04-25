#pragma once

#include "StaticMeshActor.h"

class ENGINE_API ACylinderActor : public AStaticMeshActor
{
    DECLARE_RTTI(ACylinderActor, AStaticMeshActor)
    
public:
    ACylinderActor();
    ~ACylinderActor() override = default;
    
    EBasicMeshType GetMeshType() const override { return EBasicMeshType::Cylinder; }

};
