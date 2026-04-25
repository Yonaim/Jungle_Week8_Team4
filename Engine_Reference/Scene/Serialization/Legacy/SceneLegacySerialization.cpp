#include "Engine/Scene/Serialization/Legacy/SceneLegacySerialization.h"

#include "Engine/Scene/Serialization/Common/SceneJsonConverters.h"
#include "Engine/Scene/Serialization/Runtime/ScenePropertySerialization.h"
#include "Engine/Scene/Serialization/Registry/SceneTypeRegistry.h"
#include "Engine/Component/Core/SceneComponent.h"
#include "Engine/Game/Actor.h"
#include "Engine/Scene/Scene.h"

namespace
{
    constexpr const char* SceneSchemaName = "JungleScene";
    constexpr int32       SceneSchemaVersion = 2;

    // 4주차 레거시 호환 테이블입니다. 씬 형식 개선되면 삭제해주세요.
    const TMap<FString, FString> LegacyTypeToActorName = {
        {"sphere", "ASphereActor"},
        {"cube", "ACubeActor"},
        {"triangle", "ATriangleActor"},
        {"quad", "AQuadActor"},
        {"cone", "AConeActor"},
        {"cylinder", "ACylinderActor"},
        {"ring", "ARingActor"},
        {"staticmeshcomp", "AStaticMeshActor"},
        {"staticmeshcomponent", "AStaticMeshActor"},
        {"sprite", "ASpriteActor"},
        {"text", "ATextActor"},
        {"flipbook", "AFlipbookActor"},
        {"effect", "AEffectActor"},
        {"atlassprite", "AAtlasSpriteActor"},
    };

     // SerializeLegacy용: 액터 타입 이름 → 레거시 Type 문자열
    const TMap<FString, FString> ActorTypeToLegacyType = {
        {"AStaticMeshActor", "StaticMeshComp"},
        {"ASphereActor", "Sphere"},
        {"ACubeActor", "Cube"},
        {"ATriangleActor", "Triangle"},
        {"AQuadActor", "Quad"},
        {"AConeActor", "Cone"},
        {"ACylinderActor", "Cylinder"},
        {"ARingActor", "Ring"},
        {"ASpriteActor", "Sprite"},
        {"ATextActor", "Text"},
        {"AFlipbookActor", "Flipbook"},
        {"AEffectActor", "Effect"},
        {"AAtlasSpriteActor", "AtlasSprite"},
    };


    void ApplyLegacyTransform(const FSceneJsonObject&      LegacyObj,
                              Engine::Component::USceneComponent& Component)
    {
        FVector Location = FVector::ZeroVector;
        FVector Scale = FVector::OneVector;
        FQuat   Rotation = FQuat::Identity;

        const auto LocIt = LegacyObj.find("Location");
        if (LocIt != LegacyObj.end())
        {
            Engine::Scene::Serialization::TryReadVector3(LocIt->second, Location, nullptr, "Primitive.Location");
        }

        const auto ScaleIt = LegacyObj.find("Scale");
        if (ScaleIt != LegacyObj.end())
        {
            Engine::Scene::Serialization::TryReadVector3(ScaleIt->second, Scale, nullptr, "Primitive.Scale");
        }

        const auto RotIt = LegacyObj.find("Rotation");
        if (RotIt != LegacyObj.end())
        {
            if (const auto* Arr = RotIt->second.get_ptr<const FSceneJsonArray*>())
            {
                double rx = 0.0, ry = 0.0, rz = 0.0;
                if (Arr->size() >= 3 && (*Arr)[0].is_number() && (*Arr)[1].is_number() &&
                    (*Arr)[2].is_number())
                {
                    rx = (*Arr)[0].get<double>();
                    ry = (*Arr)[1].get<double>();
                    rz = (*Arr)[2].get<double>();
                    FVector  EulerDeg(static_cast<float>(FMath::RadiansToDegrees(rx)),
                                      static_cast<float>(FMath::RadiansToDegrees(ry)),
                                      static_cast<float>(FMath::RadiansToDegrees(rz)));
                    FRotator Rotator = FRotator::MakeFromEuler(EulerDeg);
                    Rotation = Rotator.Quaternion();
                }
            }
        }

        Component.SetRelativeLocation(Location);
        Component.SetRelativeRotation(Rotation);
        Component.SetRelativeScale3D(Scale);
    }

}

namespace Engine::Scene::Serialization
{
    bool CreateActorFromLegacyComponent(const FString&                 Key,
                                        const FSceneJsonObject& LegacyObj, FScene& OutScene,
                                        FString* OutErrorMessage)
    {
        auto ToLower = [](const FString& s)
        {
            std::string t = s;
            for (char& c : t)
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return FString(t);
        };

        // Read Type
        const auto TypeIter = LegacyObj.find("Type");
        FString TypeStr;
        if (TypeIter != LegacyObj.end() && TypeIter->second.is_string())
        {
            TypeStr = TypeIter->second.get<FString>();
        }
        const FString LowerType = ToLower(TypeStr);

        const auto MapIter = LegacyTypeToActorName.find(LowerType);
        if (MapIter == LegacyTypeToActorName.end())
        {
            return false; // 매핑 없음 → 변환 불가
        }
        const FString& ActorTypeName = MapIter->second;

        // 2) Actor 생성
        bool                    bKnownActor = false;
        std::unique_ptr<AActor> Actor(
            FSceneTypeRegistry::ConstructActor(ActorTypeName, &bKnownActor));
        if (Actor == nullptr)
        {
            if (OutErrorMessage)
                *OutErrorMessage = "Failed to construct " + ActorTypeName + ".";
            return false;
        }

        try
        {
            Actor->UUID = static_cast<uint32>(std::stoul(Key));
        }
        catch (...)
        { /* keep default */
        }

        Actor->Name = ActorTypeName + " " + std::to_string(Actor->UUID);

        // 3) 루트 컴포넌트에 Transform + 프로퍼티 적용
        auto* RootComp = Actor->GetRootComponent();
        if (RootComp == nullptr)
        {
            if (OutErrorMessage)
                *OutErrorMessage = ActorTypeName + " has no root component.";
            return false;
        }

        ApplyLegacyTransform(LegacyObj, *RootComp);
        Engine::Scene::Serialization::ApplyKnownComponentProperties(LegacyObj, *RootComp);

        OutScene.AddActor(Actor.release());
        return true;
    }



    FString ResolveLegacyActorTypeName(const FString& ActorTypeName)
    {
        const auto TypeIt = ActorTypeToLegacyType.find(ActorTypeName);
        return (TypeIt != ActorTypeToLegacyType.end()) ? TypeIt->second : ActorTypeName;
    }
}
