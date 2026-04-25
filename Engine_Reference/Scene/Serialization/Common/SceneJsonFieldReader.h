#pragma once

#include "Core/CoreMinimal.h"
#include "Engine/Scene/Serialization/Json/SceneJson.h"

namespace Engine::Scene::Serialization
{
    bool TryGetObject(const FSceneJsonValue& Value, const FSceneJsonObject*& OutObject,
                      FString* OutErrorMessage, const char* Context);
    bool TryGetArray(const FSceneJsonValue& Value, const FSceneJsonArray*& OutArray,
                     FString* OutErrorMessage, const char* Context);

    const FSceneJsonValue* FindRequiredField(const FSceneJsonObject& ObjectValue,
                                             const FString& FieldName,
                                             FString* OutErrorMessage,
                                             const char* Context);
    const FSceneJsonValue* FindOptionalField(const FSceneJsonObject& ObjectValue,
                                             const FString& FieldName);

    bool TryReadStringField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                            FString& OutValue, FString* OutErrorMessage, const char* Context);
    bool TryReadBoolField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                          bool& OutValue, FString* OutErrorMessage, const char* Context);
    bool TryReadUIntField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                          uint32& OutValue, FString* OutErrorMessage, const char* Context);
    bool TryReadOptionalUIntField(const FSceneJsonObject& ObjectValue,
                                  const FString& FieldName, uint32& OutValue, bool& OutHasValue,
                                  FString* OutErrorMessage, const char* Context);
    bool TryReadIntField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                         int32& OutValue, FString* OutErrorMessage, const char* Context);
}
