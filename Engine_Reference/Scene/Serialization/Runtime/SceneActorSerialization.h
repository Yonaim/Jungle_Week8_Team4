#pragma once

#include "Core/CoreMinimal.h"
#include "Engine/Scene/Serialization/Json/SceneJson.h"

class AActor;
class FScene;

namespace Engine::Scene::Serialization
{
    const char* GetSceneSchemaName();
    int32 GetSceneSchemaVersion();

    FSceneJsonValue SerializeActor(const AActor& Actor);

    bool DeserializeActorValue(const FSceneJsonValue& ActorValue, FScene& OutScene,
                               FString* OutErrorMessage = nullptr);
}
