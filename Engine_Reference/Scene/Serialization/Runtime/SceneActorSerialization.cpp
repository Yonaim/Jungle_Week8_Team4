#include "Engine/Scene/Serialization/Runtime/SceneActorSerialization.h"

#include "Engine/Scene/Serialization/Common/SceneJsonConverters.h"
#include "Engine/Scene/Serialization/Common/SceneJsonFieldReader.h"
#include "Engine/Scene/Serialization/Runtime/ScenePropertySerialization.h"
#include "Engine/Scene/Serialization/Registry/SceneTypeRegistry.h"
#include "Engine/Scene/Serialization/Json/SceneJson.h"
#include "Engine/Component/Core/SceneComponent.h"
#include "Engine/Component/Core/UnknownComponent.h"
#include "Engine/Game/Actor.h"
#include "Engine/Game/UnknownActor.h"
#include "Engine/Scene/Scene.h"

#include <memory>

namespace
{
    constexpr const char* SceneSchemaName = "JungleScene";
    constexpr int32       SceneSchemaVersion = 2;

    FSceneJsonObject
    BuildComponentObject(const AActor&                             OwnerActor,
                         const Engine::Component::USceneComponent& Component)
    {
        FSceneJsonObject ComponentObject;

        if (Component.IsA(Engine::Component::UUnknownComponent::GetClass()))
        {
            const auto& UnknownComponent =
                static_cast<const Engine::Component::UUnknownComponent&>(Component);
            if (!UnknownComponent.GetSerializedPayload().empty())
            {
                FSceneJsonValue ParsedPayload;
                if (FSceneJsonParser::Parse(UnknownComponent.GetSerializedPayload(), ParsedPayload))
                {
                    if (const auto* ParsedObject = ParsedPayload.get_ptr<const FSceneJsonObject*>())
                    {
                        ComponentObject = *ParsedObject;
                    }
                }
            }
        }

        ComponentObject["type"] = FSceneTypeRegistry::ResolveComponentTypeName(Component);
        ComponentObject["uuid"] = static_cast<double>(Component.UUID);
        ComponentObject["name"] = Component.Name.IsValid() ? Component.Name.ToFString() : "";
        ComponentObject["is_root"] = OwnerActor.GetRootComponent() == &Component;
        if (const Engine::Component::USceneComponent* ParentComponent = Component.GetAttachParent())
        {
            ComponentObject["parent_uuid"] = static_cast<double>(ParentComponent->UUID);
        }
        else
        {
            ComponentObject.erase("parent_uuid");
        }

        FSceneJsonObject TransformObject;
        const FVector           Location = Component.GetRelativeLocation();
        const FQuat             Rotation = Component.GetRelativeQuaternion();
        const FVector           Scale = Component.GetRelativeScale3D();

        TransformObject["location"] = Engine::Scene::Serialization::MakeNumberArray({Location.X, Location.Y, Location.Z});
        TransformObject["rotation_quat"] =
            Engine::Scene::Serialization::MakeNumberArray({Rotation.X, Rotation.Y, Rotation.Z, Rotation.W});
        TransformObject["scale"] = Engine::Scene::Serialization::MakeNumberArray({Scale.X, Scale.Y, Scale.Z});
        ComponentObject["transform"] = std::move(TransformObject);

        if (!Component.IsA(Engine::Component::UUnknownComponent::GetClass()))
        {
            FSceneJsonObject PropertiesObject;
            Engine::Scene::Serialization::BuildKnownComponentProperties(
                const_cast<Engine::Component::USceneComponent&>(Component), PropertiesObject);
            ComponentObject.erase("data");
            ComponentObject["properties"] = std::move(PropertiesObject);
        }
        return ComponentObject;
    }


    FSceneJsonValue SerializeComponent(const AActor&                             OwnerActor,
                                       const Engine::Component::USceneComponent& Component)
    {
        return FSceneJsonValue(BuildComponentObject(OwnerActor, Component));
    }


    bool ApplyComponentJson(const FSceneJsonObject&      ComponentObject,
                            Engine::Component::USceneComponent& Component, FString* OutErrorMessage)
    {
        uint32 ComponentUuid = Component.UUID;
        if (!Engine::Scene::Serialization::TryReadUIntField(ComponentObject, "uuid", ComponentUuid, OutErrorMessage, "Component"))
        {
            return false;
        }
        Component.UUID = ComponentUuid;

        FString ComponentName;
        if (!Engine::Scene::Serialization::TryReadStringField(ComponentObject, "name", ComponentName, OutErrorMessage,
                                "Component"))
        {
            return false;
        }
        Component.Name = ComponentName;

        const FSceneJsonValue* TransformValue =
            Engine::Scene::Serialization::FindRequiredField(ComponentObject, "transform", OutErrorMessage, "Component");
        if (TransformValue == nullptr)
        {
            return false;
        }

        const FSceneJsonObject* TransformObject = nullptr;
        if (!Engine::Scene::Serialization::TryGetObject(*TransformValue, TransformObject, OutErrorMessage, "Component.transform"))
        {
            return false;
        }

        const FSceneJsonValue* LocationValue =
            Engine::Scene::Serialization::FindRequiredField(*TransformObject, "location", OutErrorMessage, "Component.transform");
        const FSceneJsonValue* RotationValue = Engine::Scene::Serialization::FindRequiredField(
            *TransformObject, "rotation_quat", OutErrorMessage, "Component.transform");
        const FSceneJsonValue* ScaleValue =
            Engine::Scene::Serialization::FindRequiredField(*TransformObject, "scale", OutErrorMessage, "Component.transform");
        if (LocationValue == nullptr || RotationValue == nullptr || ScaleValue == nullptr)
        {
            return false;
        }

        FVector Location = FVector::ZeroVector;
        FVector Scale = FVector::OneVector;
        FQuat   Rotation = FQuat::Identity;
        if (!Engine::Scene::Serialization::TryReadVector3(*LocationValue, Location, OutErrorMessage,
                            "Component.transform.location") ||
            !Engine::Scene::Serialization::TryReadQuat(*RotationValue, Rotation, OutErrorMessage,
                         "Component.transform.rotation_quat") ||
            !Engine::Scene::Serialization::TryReadVector3(*ScaleValue, Scale, OutErrorMessage, "Component.transform.scale"))
        {
            return false;
        }

        Component.SetRelativeLocation(Location);
        Component.SetRelativeRotation(Rotation);
        Component.SetRelativeScale3D(Scale);

        const auto PropertiesIterator = ComponentObject.find("properties");
        if (PropertiesIterator != ComponentObject.end())
        {
            if (const auto* PropertiesObject = PropertiesIterator->second.get_ptr<const FSceneJsonObject*>())
            {
                Engine::Scene::Serialization::ApplyKnownComponentProperties(*PropertiesObject, Component);
            }
            else
            {
                UE_LOG(
                    SceneIO, ELogLevel::Error,
                    "Component.properties on '%s' must be an object. Property restore was skipped.",
                    Component.GetTypeName());
            }
        }

        return true;
    }


    Engine::Component::USceneComponent*
    FindReusableComponent(const FString&                                     TypeName,
                          const TArray<Engine::Component::USceneComponent*>& ExistingComponents,
                          TArray<bool>&                                      ReusedFlags)
    {
        for (size_t Index = 0; Index < ExistingComponents.size(); ++Index)
        {
            Engine::Component::USceneComponent* ExistingComponent = ExistingComponents[Index];
            if (ExistingComponent == nullptr || ReusedFlags[Index])
            {
                continue;
            }

            if (FSceneTypeRegistry::ResolveComponentTypeName(*ExistingComponent) != TypeName)
            {
                continue;
            }

            ReusedFlags[Index] = true;
            return ExistingComponent;
        }

        return nullptr;
    }


    bool DeserializeActorValue(const FSceneJsonValue& ActorValue, FScene& OutScene,
                               FString* OutErrorMessage)
    {
        struct FPendingComponentHierarchy
        {
            Engine::Component::USceneComponent* Component = nullptr;
            bool                                bIsRootComponent = false;
            bool                                bHasParentUuid = false;
            uint32                              ParentUuid = 0;
        };

        const FSceneJsonObject* ActorObject = nullptr;
        if (!Engine::Scene::Serialization::TryGetObject(ActorValue, ActorObject, OutErrorMessage, "Actor"))
        {
            return false;
        }

        FString ActorTypeName;
        if (!Engine::Scene::Serialization::TryReadStringField(*ActorObject, "type", ActorTypeName, OutErrorMessage, "Actor"))
        {
            return false;
        }

        bool                    bKnownActorType = false;
        std::unique_ptr<AActor> Actor(
            FSceneTypeRegistry::ConstructActor(ActorTypeName, &bKnownActorType));
        if (Actor == nullptr)
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = "Failed to construct actor '" + ActorTypeName + "'.";
            }
            return false;
        }

        if (!bKnownActorType)
        {
            UE_LOG(SceneIO, ELogLevel::Error,
                   "Unknown actor type '%s' restored as placeholder.", ActorTypeName.c_str());
        }

        if (Actor->IsA(AUnknownActor::GetClass()))
        {
            auto* UnknownActor = static_cast<AUnknownActor*>(Actor.get());
            UnknownActor->SetOriginalTypeName(ActorTypeName);
            UnknownActor->SetSerializedPayload(FSceneJsonWriter::Write(ActorValue, true));
        }

        uint32 ActorUuid = Actor->UUID;
        if (!Engine::Scene::Serialization::TryReadUIntField(*ActorObject, "uuid", ActorUuid, OutErrorMessage, "Actor"))
        {
            return false;
        }
        Actor->UUID = ActorUuid;
        Actor->RefreshUUIDDebugComponent();

        FString ActorName;
        if (!Engine::Scene::Serialization::TryReadStringField(*ActorObject, "name", ActorName, OutErrorMessage, "Actor"))
        {
            return false;
        }
        Actor->Name = ActorName;

        bool bPickable = true;
        if (!Engine::Scene::Serialization::TryReadBoolField(*ActorObject, "pickable", bPickable, OutErrorMessage, "Actor"))
        {
            return false;
        }
        Actor->SetPickable(bPickable);

        const FSceneJsonValue* ComponentsValue =
            Engine::Scene::Serialization::FindRequiredField(*ActorObject, "components", OutErrorMessage, "Actor");
        if (ComponentsValue == nullptr)
        {
            return false;
        }

        const FSceneJsonArray* ComponentsArray = nullptr;
        if (!Engine::Scene::Serialization::TryGetArray(*ComponentsValue, ComponentsArray, OutErrorMessage, "Actor.components"))
        {
            return false;
        }

        const auto   ExistingComponents = Actor->GetOwnedComponents();
        TArray<bool> ReusedFlags(ExistingComponents.size(), false);
        TMap<uint32, Engine::Component::USceneComponent*> ComponentsByUuid;
        TArray<FPendingComponentHierarchy>                PendingHierarchy;
        PendingHierarchy.reserve(ComponentsArray->size());

        for (const FSceneJsonValue& ComponentValue : *ComponentsArray)
        {
            const FSceneJsonObject* ComponentObject = nullptr;
            if (!Engine::Scene::Serialization::TryGetObject(ComponentValue, ComponentObject, OutErrorMessage, "Component"))
            {
                return false;
            }

            FString ComponentTypeName;
            if (!Engine::Scene::Serialization::TryReadStringField(*ComponentObject, "type", ComponentTypeName, OutErrorMessage,
                                    "Component"))
            {
                return false;
            }

            bool bIsRootComponent = false;
            if (!Engine::Scene::Serialization::TryReadBoolField(*ComponentObject, "is_root", bIsRootComponent, OutErrorMessage,
                                  "Component"))
            {
                return false;
            }

            uint32 ParentUuid = 0;
            bool   bHasParentUuid = false;
            if (!Engine::Scene::Serialization::TryReadOptionalUIntField(*ComponentObject, "parent_uuid", ParentUuid,
                                          bHasParentUuid, OutErrorMessage, "Component"))
            {
                return false;
            }

            Engine::Component::USceneComponent* Component =
                FindReusableComponent(ComponentTypeName, ExistingComponents, ReusedFlags);
            bool bKnownComponentType = true;
            if (Component == nullptr)
            {
                Component =
                    FSceneTypeRegistry::ConstructComponent(ComponentTypeName, &bKnownComponentType);
                if (Component == nullptr)
                {
                    if (OutErrorMessage != nullptr)
                    {
                        *OutErrorMessage =
                            "Failed to construct component '" + ComponentTypeName + "'.";
                    }
                    return false;
                }

                Actor->AddOwnedComponent(Component, false);
            }

            if (!bKnownComponentType)
            {
                UE_LOG(SceneIO, ELogLevel::Error,
                       "Unknown component type '%s' restored as placeholder.",
                       ComponentTypeName.c_str());
            }

            if (Component->IsA(Engine::Component::UUnknownComponent::GetClass()))
            {
                auto* UnknownComponent =
                    static_cast<Engine::Component::UUnknownComponent*>(Component);
                UnknownComponent->SetOriginalTypeName(ComponentTypeName);
                UnknownComponent->SetSerializedPayload(
                    FSceneJsonWriter::Write(ComponentValue, true));
            }

            if (!ApplyComponentJson(*ComponentObject, *Component, OutErrorMessage))
            {
                return false;
            }

            ComponentsByUuid[Component->UUID] = Component;
            PendingHierarchy.push_back(
                FPendingComponentHierarchy{.Component = Component,
                                           .bIsRootComponent = bIsRootComponent,
                                           .bHasParentUuid = bHasParentUuid,
                                           .ParentUuid = ParentUuid});
        }

        for (const FPendingComponentHierarchy& Hierarchy : PendingHierarchy)
        {
            if (Hierarchy.Component != nullptr && Hierarchy.bIsRootComponent)
            {
                Actor->SetRootComponent(Hierarchy.Component);
            }
        }

        if (Actor->GetRootComponent() == nullptr && !Actor->GetOwnedComponents().empty())
        {
            Actor->SetRootComponent(Actor->GetOwnedComponents().front());
        }

        Engine::Component::USceneComponent* RootComponent = Actor->GetRootComponent();
        for (const FPendingComponentHierarchy& Hierarchy : PendingHierarchy)
        {
            Engine::Component::USceneComponent* Component = Hierarchy.Component;
            if (Component == nullptr || Component == RootComponent)
            {
                continue;
            }

            if (Hierarchy.bHasParentUuid)
            {
                const auto ParentIterator = ComponentsByUuid.find(Hierarchy.ParentUuid);
                if (ParentIterator != ComponentsByUuid.end() && ParentIterator->second != nullptr &&
                    ParentIterator->second != Component)
                {
                    Component->AttachToComponent(ParentIterator->second);
                    continue;
                }
            }

            if (RootComponent != nullptr)
            {
                Component->AttachToComponent(RootComponent);
            }
            else
            {
                Component->DetachFromParent();
            }
        }

        OutScene.AddActor(Actor.release());
        return true;
    }
}

namespace Engine::Scene::Serialization
{
    const char* GetSceneSchemaName() { return SceneSchemaName; }
    int32 GetSceneSchemaVersion() { return SceneSchemaVersion; }

    FSceneJsonValue SerializeActor(const AActor& Actor)
    {
        FSceneJsonObject ActorObject;

        if (Actor.IsA(AUnknownActor::GetClass()))
        {
            const auto& UnknownActor = static_cast<const AUnknownActor&>(Actor);
            if (!UnknownActor.GetSerializedPayload().empty())
            {
                FSceneJsonValue ParsedPayload;
                if (FSceneJsonParser::Parse(UnknownActor.GetSerializedPayload(), ParsedPayload))
                {
                    if (const auto* ParsedObject = ParsedPayload.get_ptr<const FSceneJsonObject*>())
                    {
                        ActorObject = *ParsedObject;
                    }
                }
            }
        }

        ActorObject["type"] = FSceneTypeRegistry::ResolveActorTypeName(Actor);
        ActorObject["uuid"] = static_cast<double>(Actor.UUID);
        ActorObject["name"] = Actor.Name.IsValid() ? Actor.Name.ToFString() : "";
        ActorObject["pickable"] = Actor.IsPickable();

        FSceneJsonArray ComponentsArray;
        const auto& Components = Actor.GetOwnedComponents();
        for (Engine::Component::USceneComponent* Component : Components)
        {
            if (Component != nullptr && Component->ShouldSerializeInScene())
            {
                ComponentsArray.push_back(SerializeComponent(Actor, *Component));
            }
        }

        ActorObject["components"] = std::move(ComponentsArray);
        return FSceneJsonValue(std::move(ActorObject));
    }

    bool DeserializeActorValue(const FSceneJsonValue& ActorValue, FScene& OutScene,
                               FString* OutErrorMessage)
    {
        return ::DeserializeActorValue(ActorValue, OutScene, OutErrorMessage);
    }

}
