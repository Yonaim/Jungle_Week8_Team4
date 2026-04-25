#include "Core/CoreMinimal.h"
#include "RingActor.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"

ARingActor::ARingActor()
{
    SetDefaultStaticMeshPath("Mesh/Primitive/Ring.obj");
    Name = "RingActor";
}

REGISTER_CLASS(, ARingActor)
