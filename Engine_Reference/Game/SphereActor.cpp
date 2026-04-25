#include "Core/CoreMinimal.h"
#include "SphereActor.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"

ASphereActor::ASphereActor()
{
    SetDefaultStaticMeshPath("Mesh/Primitive/Sphere.obj");
    Name = "SphereActor";
}
REGISTER_CLASS(, ASphereActor)