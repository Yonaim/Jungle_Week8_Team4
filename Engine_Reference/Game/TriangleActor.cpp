#include "TriangleActor.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"

#include "Engine/Component/Core/PrimitiveComponent.h"

ATriangleActor::ATriangleActor()
{
    SetDefaultStaticMeshPath("Mesh/Primitive/Triangle.obj");
    Name = "TriangleActor";
}

REGISTER_CLASS(, ATriangleActor)
