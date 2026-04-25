#pragma once

#include "Core/CoreMinimal.h"
#include "Engine/Scene/Serialization/Json/SceneJson.h"
#include "Core/Math/Vector4.h"

namespace Engine::Scene::Serialization
{
    FSceneJsonValue MakeNumberArray(std::initializer_list<double> Values);

    bool TryReadIntValue(const FSceneJsonValue& Value, int32& OutValue, FString* OutErrorMessage,
                         const char* Context);
    bool TryReadFloatValue(const FSceneJsonValue& Value, float& OutValue,
                           FString* OutErrorMessage, const char* Context);
    bool TryReadVector3(const FSceneJsonValue& Value, FVector& OutVector, FString* OutErrorMessage,
                        const char* Context);
    bool TryReadQuat(const FSceneJsonValue& Value, FQuat& OutQuat, FString* OutErrorMessage,
                     const char* Context);
    bool TryReadColor(const FSceneJsonValue& Value, FVector4& OutColor, FString* OutErrorMessage,
                      const char* Context);
}
