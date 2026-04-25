#pragma once

#include "Core/CoreMinimal.h"
#include "Engine/Scene/Serialization/Json/SceneJson.h"

class FScene;

namespace Engine::Scene::Serialization
{
    bool CreateActorFromLegacyComponent(const FString& Key,
                                        const FSceneJsonObject& LegacyObj,
                                        FScene& OutScene,
                                        FString* OutErrorMessage = nullptr);

    FString ResolveLegacyActorTypeName(const FString& ActorTypeName);
}
