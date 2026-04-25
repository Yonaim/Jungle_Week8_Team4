#pragma once

#include "Core/CoreMinimal.h"

class FScene;
class AActor;

class FAssetObjectManager;

namespace Engine::Component
{
    class USceneComponent;
}

class ENGINE_API FSceneAssetBinder
{
  public:
    static void BindScene(FScene* InScene, FAssetObjectManager* InAssetObjectManager);
    static void BindActor(AActor* InActor, FAssetObjectManager* InAssetObjectManager);
    static void BindComponent(Engine::Component::USceneComponent* InComponent,
                              FAssetObjectManager* InAssetObjectManager);
};
