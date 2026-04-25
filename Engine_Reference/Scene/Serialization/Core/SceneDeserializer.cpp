#include "Engine/Scene/Serialization/Core/SceneDeserializer.h"

#include "Engine/Scene/Serialization/Json/SceneJson.h"
#include "Engine/Scene/Serialization/Common/SceneJsonFieldReader.h"
#include "Engine/Scene/Serialization/Runtime/SceneActorSerialization.h"
#include "Engine/Scene/Serialization/Legacy/SceneLegacySerialization.h"
#include "Engine/Scene/Scene.h"

#include <fstream>
#include <memory>

std::unique_ptr<FScene> FSceneDeserializer::Deserialize(const FString& JsonSource,
                                                        FString*       OutErrorMessage)
{
    FSceneJsonValue RootValue;
    if (!FSceneJsonParser::Parse(JsonSource, RootValue, OutErrorMessage))
    {
        return nullptr;
    }

    const FSceneJsonObject* RootObject = nullptr;
    if (!Engine::Scene::Serialization::TryGetObject(RootValue, RootObject, OutErrorMessage, "Scene"))
    {
        return nullptr;
    }

    int32 Version = 0;
    if (!Engine::Scene::Serialization::TryReadIntField(*RootObject, "version", Version,
                                                   OutErrorMessage, "Scene"))
    {
        return nullptr;
    }

    if (Version != Engine::Scene::Serialization::GetSceneSchemaVersion())
    {
        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = "Unsupported scene version '" + std::to_string(Version) + "'.";
        }
        return nullptr;
    }

    std::unique_ptr<FScene> Scene = std::make_unique<FScene>();

    if (const FSceneJsonValue* ActorsValue =
            Engine::Scene::Serialization::FindOptionalField(*RootObject, "actors"))
    {
        const FSceneJsonArray* ActorsArray = nullptr;
        if (!Engine::Scene::Serialization::TryGetArray(*ActorsValue, ActorsArray, OutErrorMessage,
                                                   "Scene.actors"))
        {
            return nullptr;
        }

        for (const FSceneJsonValue& ActorValue : *ActorsArray)
        {
            FString ActorErrorMessage;
            if (!Engine::Scene::Serialization::DeserializeActorValue(ActorValue, *Scene,
                                                                 &ActorErrorMessage))
            {
                if (OutErrorMessage != nullptr)
                {
                    *OutErrorMessage = ActorErrorMessage;
                }
                return nullptr;
            }
        }
    }

    if (const FSceneJsonValue* PrimitivesValue =
            Engine::Scene::Serialization::FindOptionalField(*RootObject, "Primitives"))
    {
        const FSceneJsonObject* PrimitivesObject = nullptr;
        if (Engine::Scene::Serialization::TryGetObject(*PrimitivesValue, PrimitivesObject,
                                                   OutErrorMessage, "Scene.Primitives"))
        {
            for (const auto& Pair : *PrimitivesObject)
            {
                const FSceneJsonObject* LegacyObj = nullptr;
                if (!Engine::Scene::Serialization::TryGetObject(Pair.second, LegacyObj,
                                                            OutErrorMessage,
                                                            "Scene.Primitives.item"))
                {
                    return nullptr;
                }

                FString LocalError;
                if (!Engine::Scene::Serialization::CreateActorFromLegacyComponent(
                        Pair.first, *LegacyObj, *Scene, &LocalError))
                {
                    UE_LOG(SceneIO, ELogLevel::Warning,
                           "Skipped legacy primitive '%s': %s", Pair.first.c_str(),
                           LocalError.empty() ? "unsupported primitive" : LocalError.c_str());
                    continue;
                }
            }
        }
    }

    if (const FSceneJsonValue* PrimitivesValue =
        Engine::Scene::Serialization::FindOptionalField(*RootObject, "PerspectiveCamera"))
    {
        const FSceneJsonObject* CameraObject = nullptr;
        if (Engine::Scene::Serialization::TryGetObject(*PrimitivesValue, CameraObject,
                                                   OutErrorMessage, "Scene.PerspectiveCamera"))
        {
            FCameraInfo CameraInfo;
            if (DeserializeCameraInfo(*CameraObject, CameraInfo, OutErrorMessage))
            {
                Scene->SetEditorCameraInfo(CameraInfo);
            }
        }
       }

    return Scene;
}

std::unique_ptr<FScene> FSceneDeserializer::LoadFromFile(const std::filesystem::path& FilePath,
                                                         FString* OutErrorMessage)
{
    std::ifstream File(FilePath, std::ios::binary);
    if (!File.is_open())
    {
        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = "Failed to open scene file.";
        }
        return nullptr;
    }

    FString JsonSource((std::istreambuf_iterator<char>(File)), std::istreambuf_iterator<char>());
    if (JsonSource.empty() && File.bad())
    {
        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = "Failed to read scene file.";
        }
        return nullptr;
    }

    return Deserialize(JsonSource, OutErrorMessage);
}
