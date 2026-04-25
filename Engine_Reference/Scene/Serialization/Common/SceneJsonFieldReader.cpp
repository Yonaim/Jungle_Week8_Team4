#include "Engine/Scene/Serialization/Common/SceneJsonFieldReader.h"

namespace Engine::Scene::Serialization
{
    const FSceneJsonValue* FindRequiredField(const FSceneJsonObject& ObjectValue,
                                             const FString& FieldName, FString* OutErrorMessage,
                                             const char* Context)
    {
        const auto Iterator = ObjectValue.find(FieldName);
        if (Iterator != ObjectValue.end())
        {
            return &Iterator->second;
        }

        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = FString(Context) + " is missing required field '" + FieldName + "'.";
        }
        return nullptr;
    }

    const FSceneJsonValue* FindOptionalField(const FSceneJsonObject& ObjectValue,
                                             const FString& FieldName)
    {
        const auto Iterator = ObjectValue.find(FieldName);
        if (Iterator != ObjectValue.end())
        {
            return &Iterator->second;
        }
        return nullptr;
    }

    bool TryGetObject(const FSceneJsonValue& Value, const FSceneJsonObject*& OutObject,
                      FString* OutErrorMessage, const char* Context)
    {
        OutObject = Value.get_ptr<const FSceneJsonObject*>();
        if (OutObject != nullptr)
        {
            return true;
        }

        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = FString(Context) + " must be an object.";
        }
        return false;
    }

    bool TryGetArray(const FSceneJsonValue& Value, const FSceneJsonArray*& OutArray,
                     FString* OutErrorMessage, const char* Context)
    {
        OutArray = Value.get_ptr<const FSceneJsonArray*>();
        if (OutArray != nullptr)
        {
            return true;
        }

        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = FString(Context) + " must be an array.";
        }
        return false;
    }

    bool TryReadStringField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                            FString& OutValue, FString* OutErrorMessage, const char* Context)
    {
        const FSceneJsonValue* FieldValue =
            FindRequiredField(ObjectValue, FieldName, OutErrorMessage, Context);
        if (FieldValue == nullptr)
        {
            return false;
        }

        if (FieldValue->is_string())
        {
            OutValue = FieldValue->get<FString>();
            return true;
        }

        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = FString(Context) + "." + FieldName + " must be a string.";
        }
        return false;
    }

    bool TryReadBoolField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                          bool& OutValue, FString* OutErrorMessage, const char* Context)
    {
        const FSceneJsonValue* FieldValue =
            FindRequiredField(ObjectValue, FieldName, OutErrorMessage, Context);
        if (FieldValue == nullptr)
        {
            return false;
        }

        if (FieldValue->is_boolean())
        {
            OutValue = FieldValue->get<bool>();
            return true;
        }

        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = FString(Context) + "." + FieldName + " must be a bool.";
        }
        return false;
    }

    bool TryReadUIntField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                          uint32& OutValue, FString* OutErrorMessage, const char* Context)
    {
        const FSceneJsonValue* FieldValue =
            FindRequiredField(ObjectValue, FieldName, OutErrorMessage, Context);
        if (FieldValue == nullptr)
        {
            return false;
        }

        if (!FieldValue->is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage =
                    FString(Context) + "." + FieldName + " must be a non-negative number.";
            }
            return false;
        }

        const double NumberValue = FieldValue->get<double>();
        if (NumberValue < 0.0)
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage =
                    FString(Context) + "." + FieldName + " must be a non-negative number.";
            }
            return false;
        }

        OutValue = static_cast<uint32>(NumberValue);
        return true;
    }

    bool TryReadOptionalUIntField(const FSceneJsonObject& ObjectValue,
                                  const FString& FieldName, uint32& OutValue, bool& OutHasValue,
                                  FString* OutErrorMessage, const char* Context)
    {
        const auto Iterator = ObjectValue.find(FieldName);
        if (Iterator == ObjectValue.end())
        {
            OutHasValue = false;
            return true;
        }

        if (!Iterator->second.is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage =
                    FString(Context) + "." + FieldName + " must be a non-negative number.";
            }
            return false;
        }

        const double NumberValue = Iterator->second.get<double>();
        if (NumberValue < 0.0)
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage =
                    FString(Context) + "." + FieldName + " must be a non-negative number.";
            }
            return false;
        }

        OutValue = static_cast<uint32>(NumberValue);
        OutHasValue = true;
        return true;
    }

    bool TryReadIntField(const FSceneJsonObject& ObjectValue, const FString& FieldName,
                         int32& OutValue, FString* OutErrorMessage, const char* Context)
    {
        const FSceneJsonValue* FieldValue =
            FindRequiredField(ObjectValue, FieldName, OutErrorMessage, Context);
        if (FieldValue == nullptr)
        {
            return false;
        }

        if (!FieldValue->is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + "." + FieldName + " must be a number.";
            }
            return false;
        }

        OutValue = static_cast<int32>(FieldValue->get<double>());
        return true;
    }
}
