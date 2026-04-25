#include "Engine/Scene/Serialization/Runtime/ScenePropertySerialization.h"

#include "Engine/Scene/Serialization/Common/SceneJsonConverters.h"
#include "Engine/Component/Core/ComponentProperty.h"
#include "Engine/Component/Core/SceneComponent.h"
#include "Engine/Scene/SceneAssetPath.h"

namespace Engine::Scene::Serialization
{
    FSceneJsonValue
    SerializeComponentProperty(const Engine::Component::FComponentPropertyDescriptor& Descriptor)
    {
        using namespace Engine::Component;

        switch (Descriptor.Type)
        {
        case EComponentPropertyType::Bool:
            return Descriptor.BoolGetter ? FSceneJsonValue(Descriptor.BoolGetter()) : FSceneJsonValue(false);

        case EComponentPropertyType::Int:
            return static_cast<double>(Descriptor.IntGetter ? Descriptor.IntGetter() : 0);

        case EComponentPropertyType::Float:
            return static_cast<double>(Descriptor.FloatGetter ? Descriptor.FloatGetter() : 0.0f);

        case EComponentPropertyType::String:
            return Descriptor.StringGetter ? FSceneJsonValue(Descriptor.StringGetter()) : FSceneJsonValue(FString{});

        case EComponentPropertyType::AssetPath:
        {
            const FString AssetPath = Descriptor.StringGetter ? Descriptor.StringGetter() : FString{};
            return NormalizeSceneAssetPath(AssetPath);
        }

        case EComponentPropertyType::Vector3:
        {
            const FVector Value = Descriptor.VectorGetter ? Descriptor.VectorGetter() : FVector::ZeroVector;
            return MakeNumberArray({Value.X, Value.Y, Value.Z});
        }

        case EComponentPropertyType::Color:
        {
            const FColor Value = Descriptor.ColorGetter ? Descriptor.ColorGetter() : FColor::White();
            return MakeNumberArray({Value.r, Value.g, Value.b, Value.a});
        }
        }

        return nullptr;
    }

    void BuildKnownComponentProperties(Engine::Component::USceneComponent& Component,
                                       FSceneJsonObject& OutPropertiesObject)
    {
        Engine::Component::FComponentPropertyBuilder Builder;
        Component.DescribeProperties(Builder);

        for (const Engine::Component::FComponentPropertyDescriptor& Descriptor : Builder.GetProperties())
        {
            if (!Descriptor.bSerializeInScene || Descriptor.Key.empty())
            {
                continue;
            }

            OutPropertiesObject[Descriptor.Key] = SerializeComponentProperty(Descriptor);
        }
    }

    void LogComponentPropertyRestoreFailure(const Engine::Component::USceneComponent& Component,
                                            const FString& PropertyKey, const FString& ErrorMessage)
    {
        UE_LOG(SceneIO, ELogLevel::Error,
               "Failed to restore property '%s' on component '%s': %s", PropertyKey.c_str(),
               Component.GetTypeName(), ErrorMessage.c_str());
    }

    void ApplyKnownComponentProperties(const FSceneJsonObject& PropertiesObject,
                                       Engine::Component::USceneComponent& Component)
    {
        using namespace Engine::Component;

        FComponentPropertyBuilder Builder;
        Component.DescribeProperties(Builder);

        for (const FComponentPropertyDescriptor& Descriptor : Builder.GetProperties())
        {
            if (!Descriptor.bSerializeInScene || Descriptor.Key.empty())
            {
                continue;
            }

            const auto PropertyIterator = PropertiesObject.find(Descriptor.Key);
            if (PropertyIterator == PropertiesObject.end())
            {
                continue;
            }

            const FSceneJsonValue& PropertyValue = PropertyIterator->second;
            FString PropertyError;
            const FString Context = "Component.properties." + Descriptor.Key;

            switch (Descriptor.Type)
            {
            case EComponentPropertyType::Bool:
            {
                if (!PropertyValue.is_boolean())
                {
                    PropertyError = Context + " must be a bool.";
                    break;
                }

                if (Descriptor.BoolSetter)
                {
                    Descriptor.BoolSetter(PropertyValue.get<bool>());
                }
                continue;
            }

            case EComponentPropertyType::Int:
            {
                int32 Value = 0;
                if (!TryReadIntValue(PropertyValue, Value, &PropertyError, Context.c_str()))
                {
                    break;
                }

                if (Descriptor.IntSetter)
                {
                    Descriptor.IntSetter(Value);
                }
                continue;
            }

            case EComponentPropertyType::Float:
            {
                float Value = 0.0f;
                if (!TryReadFloatValue(PropertyValue, Value, &PropertyError, Context.c_str()))
                {
                    break;
                }

                if (Descriptor.FloatSetter)
                {
                    Descriptor.FloatSetter(Value);
                }
                continue;
            }

            case EComponentPropertyType::String:
            {
                if (!PropertyValue.is_string())
                {
                    PropertyError = Context + " must be a string.";
                    break;
                }

                if (Descriptor.StringSetter)
                {
                    Descriptor.StringSetter(PropertyValue.get<FString>());
                }
                continue;
            }

            case EComponentPropertyType::AssetPath:
            {
                if (!PropertyValue.is_string())
                {
                    PropertyError = Context + " must be a string.";
                    break;
                }

                if (Descriptor.StringSetter)
                {
                    Descriptor.StringSetter(NormalizeSceneAssetPath(PropertyValue.get<FString>()));
                }
                continue;
            }

            case EComponentPropertyType::Vector3:
            {
                FVector Value = FVector::ZeroVector;
                if (!TryReadVector3(PropertyValue, Value, &PropertyError, Context.c_str()))
                {
                    break;
                }

                if (Descriptor.VectorSetter)
                {
                    Descriptor.VectorSetter(Value);
                }
                continue;
            }

            case EComponentPropertyType::Color:
            {
                FVector4 Value = FVector4::Zero();
                if (!TryReadColor(PropertyValue, Value, &PropertyError, Context.c_str()))
                {
                    break;
                }

                if (Descriptor.ColorSetter)
                {
                    Descriptor.ColorSetter(FColor(Value.X, Value.Y, Value.Z, Value.W));
                }
                continue;
            }
            }

            LogComponentPropertyRestoreFailure(Component, Descriptor.Key, PropertyError);
        }
    }
}
