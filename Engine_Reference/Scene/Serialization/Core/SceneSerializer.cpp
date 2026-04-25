#include "Engine/Scene/Serialization/Core/SceneSerializer.h"

#include "Engine/Scene/Serialization/Json/SceneJson.h"
#include "Engine/Scene/Serialization/Runtime/SceneActorSerialization.h"
#include "Engine/Scene/Serialization/Legacy/SceneLegacySerialization.h"
#include "Engine/Scene/Serialization/Runtime/ScenePropertySerialization.h"
#include "Engine/Scene/Serialization/Registry/SceneTypeRegistry.h"
#include "Engine/Scene/CameraInfo.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Game/Actor.h"
#include "Engine/Scene/Serialization/Common/SceneJsonConverters.h"

#include <fstream>

bool FSceneSerializer::Serialize(const FScene& Scene, FString& OutJson, FString* OutErrorMessage)
{
    (void)OutErrorMessage;

    FSceneJsonObject RootObject;
    RootObject["schema"] = Engine::Scene::Serialization::GetSceneSchemaName();
    RootObject["version"] =
        static_cast<double>(Engine::Scene::Serialization::GetSceneSchemaVersion());

    FSceneJsonArray        ActorsArray;
    const TArray<AActor*>& Actors = Scene.GetActors();
    ActorsArray.reserve(Actors.size());
    for (AActor* Actor : Actors)
    {
        if (Actor != nullptr)
        {
            ActorsArray.push_back(Engine::Scene::Serialization::SerializeActor(*Actor));
        }
    }

    RootObject["actors"] = std::move(ActorsArray);
    OutJson = FSceneJsonWriter::Write(FSceneJsonValue(std::move(RootObject)), true);
    return true;
}

bool FSceneSerializer::SaveToFile(const FScene& Scene, const FCameraInfo& CameraInfo,
                                  const std::filesystem::path& FilePath, FString* OutErrorMessage)
{
    FString JsonText;
    if (!SerializeLegacy(Scene, CameraInfo, JsonText, OutErrorMessage))
    {
        return false;
    }

    std::error_code ErrorCode;
    std::filesystem::create_directories(FilePath.parent_path(), ErrorCode);

    std::ofstream File(FilePath, std::ios::binary | std::ios::trunc);
    if (!File.is_open())
    {
        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = "Failed to open scene file for writing.";
        }
        return false;
    }

    File.write(JsonText.data(), static_cast<std::streamsize>(JsonText.size()));
    if (!File.good())
    {
        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = "Failed to write scene file.";
        }
        return false;
    }

    return true;
}

bool FSceneSerializer::SerializeLegacy(const FScene& Scene, const FCameraInfo& CameraInfo,
                                       FString& OutJson, FString* OutErrorMessage)
{
    (void)OutErrorMessage;

    FSceneJsonObject RootObject;
    RootObject["schema"] = Engine::Scene::Serialization::GetSceneSchemaName();
    RootObject["version"] =
        static_cast<double>(Engine::Scene::Serialization::GetSceneSchemaVersion());

    FSceneJsonObject PrimitivesObject;
    uint32           MaxUUID = 0;

    const TArray<AActor*>& Actors = Scene.GetActors();
    for (AActor* Actor : Actors)
    {
        if (Actor == nullptr)
        {
            continue;
        }

        auto* RootComp = Actor->GetRootComponent();
        if (RootComp == nullptr)
        {
            continue;
        }

        const FString Key = std::to_string(Actor->UUID);
        if (Actor->UUID > MaxUUID)
        {
            MaxUUID = Actor->UUID;
        }

        FSceneJsonObject EntryObject;
        EntryObject["Type"] = Engine::Scene::Serialization::ResolveLegacyActorTypeName(
            FSceneTypeRegistry::ResolveActorTypeName(*Actor));

        const FVector Location = RootComp->GetRelativeLocation();
        const FQuat   Rotation = RootComp->GetRelativeQuaternion();
        const FVector Scale = RootComp->GetRelativeScale3D();

        EntryObject["Location"] =
            Engine::Scene::Serialization::MakeNumberArray({Location.X, Location.Y, Location.Z});

        FRotator Rotator(Rotation);
        FVector  EulerDeg = Rotator.Euler();
        EntryObject["Rotation"] = Engine::Scene::Serialization::MakeNumberArray(
            {FMath::DegreesToRadians(EulerDeg.X), FMath::DegreesToRadians(EulerDeg.Y),
             FMath::DegreesToRadians(EulerDeg.Z)});

        EntryObject["Scale"] =
            Engine::Scene::Serialization::MakeNumberArray({Scale.X, Scale.Y, Scale.Z});

        Engine::Scene::Serialization::BuildKnownComponentProperties(*RootComp, EntryObject);
        PrimitivesObject[Key] = std::move(EntryObject);
    }

    RootObject["NextUUID"] = static_cast<double>(MaxUUID + 1);
    RootObject["Primitives"] = std::move(PrimitivesObject);

    {   //Camera Serialize
        FSceneJsonObject CameraObject;
        const FVector    CamLoc = CameraInfo.Location;
        const FRotator   CamRot = CameraInfo.Rotation;
        CameraObject["Location"] =
            Engine::Scene::Serialization::MakeNumberArray({CamLoc.X, CamLoc.Y, CamLoc.Z});
        CameraObject["Rotation"] =
            Engine::Scene::Serialization::MakeNumberArray({CamRot.Pitch, CamRot.Yaw, CamRot.Roll});
        CameraObject["FOV"] = CameraInfo.FOV;
        CameraObject["NearClip"] = CameraInfo.NearClip;
        CameraObject["FarClip"] = CameraInfo.FarClip;
        RootObject["PerspectiveCamera"] = std::move(CameraObject);
    }
    
    OutJson = FSceneJsonWriter::Write(FSceneJsonValue(std::move(RootObject)), true);

    return true;
}
