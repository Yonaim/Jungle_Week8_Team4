#include "Engine/Scene/Scene.h"

#include <algorithm>

#include "Engine/Component/Core/PrimitiveComponent.h"
#include "Engine/Component/Mesh/LineBatchComponent.h"
#include "Engine/Game/Actor.h"

FScene::FScene()
{
}

FScene::~FScene()
{
    Clear();
}

bool FScene::RemoveActor(AActor* InActor)
{
    if (InActor == nullptr)
    {
        return false;
    }

    for (auto Iterator = Actors.begin(); Iterator != Actors.end(); ++Iterator)
    {
        if (*Iterator != InActor)
        {
            continue;
        }

    delete InActor;
    Actors.erase(Iterator);
    return true;
    }

    return false;
}

void FScene::Clear()
{
    for (AActor* Actor : Actors)
    {
        delete Actor;
    }
    Actors.clear();
}

void FScene::Tick(float DeltaTime)
{
    for (auto& actor : Actors)
    {
        if (actor)
        {
            actor->Tick(DeltaTime);
        }
    }
}

void FScene::BuildRenderData(FSceneRenderData& OutRenderData,
                             ESceneShowFlags   InShowFlags) const
{
    for (AActor* Actor : Actors)
    {
        if (Actor == nullptr)
        {
            continue;
        }

        for (Engine::Component::USceneComponent* Component : Actor->GetOwnedComponents())
        {
            if (auto* PrimitiveComponent = Cast<Engine::Component::UPrimitiveComponent>(Component))
            {
                PrimitiveComponent->CollectRenderData(OutRenderData, InShowFlags);
            }
        }
    }
}