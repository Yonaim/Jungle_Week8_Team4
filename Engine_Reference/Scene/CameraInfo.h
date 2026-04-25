#pragma once
#include "Engine/Scene/Serialization/Json/SceneJson.h"

struct FCameraInfo
{
    FVector  Location;
    FRotator Rotation;
    float    FOV = 60.f;
    float    NearClip = 0.1f;
    float    FarClip = 1000.f;
};

bool DeserializeCameraInfo(const FSceneJsonObject& CameraObject, FCameraInfo& OutCameraInfo,
                           FString* OutErrorMessage = nullptr);