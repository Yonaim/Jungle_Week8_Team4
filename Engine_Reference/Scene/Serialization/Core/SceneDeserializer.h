#pragma once

#include "Core/CoreMinimal.h"

#include <filesystem>
#include <memory>

class FScene;

class ENGINE_API FSceneDeserializer
{
  public:
    static std::unique_ptr<FScene> Deserialize(const FString& JsonSource,
                                               FString* OutErrorMessage = nullptr);
    static std::unique_ptr<FScene> LoadFromFile(const std::filesystem::path& FilePath,
                                                FString* OutErrorMessage = nullptr);
};
