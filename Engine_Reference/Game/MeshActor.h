#pragma once
#include "Actor.h"

class ENGINE_API AMeshActor : public AActor
{
    DECLARE_RTTI(AMeshActor, AActor)
	public:
	AMeshActor() = default;
	~AMeshActor() override = default;
};

