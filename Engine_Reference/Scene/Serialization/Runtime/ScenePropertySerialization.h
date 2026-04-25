#pragma once

#include "Core/CoreMinimal.h"
#include "Engine/Scene/Serialization/Json/SceneJson.h"

namespace Engine::Component
{
    class USceneComponent;
}

namespace Engine::Scene::Serialization
{
    void BuildKnownComponentProperties(Engine::Component::USceneComponent& Component,
                                       FSceneJsonObject& OutPropertiesObject);
    void ApplyKnownComponentProperties(const FSceneJsonObject& PropertiesObject,
                                       Engine::Component::USceneComponent& Component);
}
