#pragma once

#include "Core/CoreMinimal.h"

#include <filesystem>

class FScene;
struct FCameraInfo;

class ENGINE_API FSceneSerializer
{
  public:
    static bool Serialize(const FScene& Scene, FString& OutJson,
                          FString* OutErrorMessage = nullptr);
    static bool SaveToFile(const FScene& Scene, const FCameraInfo& CameraInfo,
                           const std::filesystem::path& FilePath,
                           FString*                     OutErrorMessage = nullptr);

    // 4주차 용 레거시 씬 형식 직렬화 함수입니다. 개선된 씬 형식이 도입되면 제거해주세요.
    static bool SerializeLegacy(const FScene& Scene, const FCameraInfo& CameraInfo,
                                FString& OutJson, FString* OutErrorMessage = nullptr);
};
