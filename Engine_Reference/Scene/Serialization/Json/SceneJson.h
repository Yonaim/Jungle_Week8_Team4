#pragma once

#include "Core/CoreMinimal.h"

#include <ThirdParty/nlohmann/json.hpp>

using FSceneJsonValue = nlohmann::json;
using FSceneJsonObject = nlohmann::json::object_t;
using FSceneJsonArray = nlohmann::json::array_t;

class ENGINE_API FSceneJsonParser
{
  public:
    static bool Parse(const FString& Source, FSceneJsonValue& OutValue,
                      FString* OutErrorMessage = nullptr);
};

class ENGINE_API FSceneJsonWriter
{
  public:
    static FString Write(const FSceneJsonValue& Value, bool bPretty = true);
};
