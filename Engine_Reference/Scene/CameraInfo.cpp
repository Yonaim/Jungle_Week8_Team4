#include "Core/CoreMinimal.h"
#include "CameraInfo.h"

#include "Engine/Scene/Serialization/Common/SceneJsonConverters.h"
#include "Engine/Scene/Serialization/Common/SceneJsonFieldReader.h"

bool DeserializeCameraInfo(const FSceneJsonObject& CameraObject, FCameraInfo& OutCameraInfo,
                           FString* OutErrorMessage)
{
    // Location
    const FSceneJsonValue* LocationValue = Engine::Scene::Serialization::FindRequiredField(
        CameraObject, "Location", OutErrorMessage, "Camera");
    if (!LocationValue ||
        !Engine::Scene::Serialization::TryReadVector3(*LocationValue, OutCameraInfo.Location,
                                                      OutErrorMessage, "Camera.Location"))
        return false;

    // Rotation
    const FSceneJsonValue* RotationValue = Engine::Scene::Serialization::FindRequiredField(
        CameraObject, "Rotation", OutErrorMessage, "Camera");
    if (!RotationValue || !Engine::Scene::Serialization::TryReadVector3(
                              *RotationValue, reinterpret_cast<FVector&>(OutCameraInfo.Rotation),
                              OutErrorMessage, "Camera.Rotation"))
        return false;

    // FOV
    const FSceneJsonValue* FOVValue = Engine::Scene::Serialization::FindRequiredField(
        CameraObject, "FOV", OutErrorMessage, "Camera");
    if (!FOVValue || !Engine::Scene::Serialization::TryReadFloatValue(
                         *FOVValue, OutCameraInfo.FOV, OutErrorMessage, "Camera.FOV"))
        return false;

    // NearClip
    const FSceneJsonValue* NearClipValue = Engine::Scene::Serialization::FindRequiredField(
        CameraObject, "NearClip", OutErrorMessage, "Camera");
    if (!NearClipValue ||
        !Engine::Scene::Serialization::TryReadFloatValue(*NearClipValue, OutCameraInfo.NearClip,
                                                         OutErrorMessage, "Camera.NearClip"))
        return false;

    // FarClip
    const FSceneJsonValue* FarClipValue = Engine::Scene::Serialization::FindRequiredField(
        CameraObject, "FarClip", OutErrorMessage, "Camera");
    if (!FarClipValue ||
        !Engine::Scene::Serialization::TryReadFloatValue(*FarClipValue, OutCameraInfo.FarClip,
                                                         OutErrorMessage, "Camera.FarClip"))
        return false;

    return true;
}
