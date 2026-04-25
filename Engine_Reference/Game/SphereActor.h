#pragma once
#include "StaticMeshActor.h"


class ENGINE_API ASphereActor : public AStaticMeshActor
{
	DECLARE_RTTI(ASphereActor, AStaticMeshActor)

public:
	ASphereActor();
	~ASphereActor() override = default;

	EBasicMeshType GetMeshType() const override { return EBasicMeshType::Sphere; }
};

