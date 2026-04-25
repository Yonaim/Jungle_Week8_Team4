#include "Engine/Scene/Serialization/Common/SceneJsonConverters.h"

#include "Engine/Scene/Serialization/Common/SceneJsonFieldReader.h"

namespace Engine::Scene::Serialization
{
    FSceneJsonValue MakeNumberArray(std::initializer_list<double> Values)
    {
        FSceneJsonValue ArrayValue = FSceneJsonValue::array();
        for (double Value : Values)
        {
            ArrayValue.push_back(Value);
        }
        return ArrayValue;
    }

    bool TryReadIntValue(const FSceneJsonValue& Value, int32& OutValue, FString* OutErrorMessage,
                         const char* Context)
    {
        if (!Value.is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must be a number.";
            }
            return false;
        }

        OutValue = static_cast<int32>(Value.get<double>());
        return true;
    }

    bool TryReadFloatValue(const FSceneJsonValue& Value, float& OutValue, FString* OutErrorMessage,
                           const char* Context)
    {
        if (!Value.is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must be a number.";
            }
            return false;
        }

        OutValue = static_cast<float>(Value.get<double>());
        return true;
    }

    bool TryReadVector3(const FSceneJsonValue& Value, FVector& OutVector, FString* OutErrorMessage,
                        const char* Context)
    {
        const FSceneJsonArray* ArrayValue = nullptr;
        if (!TryGetArray(Value, ArrayValue, OutErrorMessage, Context))
        {
            return false;
        }

        if (ArrayValue->size() != 3)
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must have exactly 3 numbers.";
            }
            return false;
        }

        if (!(*ArrayValue)[0].is_number() || !(*ArrayValue)[1].is_number() ||
            !(*ArrayValue)[2].is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must contain only numbers.";
            }
            return false;
        }

        OutVector = FVector(static_cast<float>((*ArrayValue)[0].get<double>()),
                            static_cast<float>((*ArrayValue)[1].get<double>()),
                            static_cast<float>((*ArrayValue)[2].get<double>()));
        return true;
    }

    bool TryReadQuat(const FSceneJsonValue& Value, FQuat& OutQuat, FString* OutErrorMessage,
                     const char* Context)
    {
        const FSceneJsonArray* ArrayValue = nullptr;
        if (!TryGetArray(Value, ArrayValue, OutErrorMessage, Context))
        {
            return false;
        }

        if (ArrayValue->size() != 4)
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must have exactly 4 numbers.";
            }
            return false;
        }

        if (!(*ArrayValue)[0].is_number() || !(*ArrayValue)[1].is_number() ||
            !(*ArrayValue)[2].is_number() || !(*ArrayValue)[3].is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must contain only numbers.";
            }
            return false;
        }

        OutQuat = FQuat(static_cast<float>((*ArrayValue)[0].get<double>()),
                        static_cast<float>((*ArrayValue)[1].get<double>()),
                        static_cast<float>((*ArrayValue)[2].get<double>()),
                        static_cast<float>((*ArrayValue)[3].get<double>()));
        OutQuat.Normalize();
        return true;
    }

    bool TryReadColor(const FSceneJsonValue& Value, FVector4& OutColor, FString* OutErrorMessage,
                      const char* Context)
    {
        const FSceneJsonArray* ArrayValue = nullptr;
        if (!TryGetArray(Value, ArrayValue, OutErrorMessage, Context))
        {
            return false;
        }

        if (ArrayValue->size() != 4)
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must have exactly 4 numbers.";
            }
            return false;
        }

        if (!(*ArrayValue)[0].is_number() || !(*ArrayValue)[1].is_number() ||
            !(*ArrayValue)[2].is_number() || !(*ArrayValue)[3].is_number())
        {
            if (OutErrorMessage != nullptr)
            {
                *OutErrorMessage = FString(Context) + " must contain only numbers.";
            }
            return false;
        }

        OutColor = FVector4(static_cast<float>((*ArrayValue)[0].get<double>()),
                            static_cast<float>((*ArrayValue)[1].get<double>()),
                            static_cast<float>((*ArrayValue)[2].get<double>()),
                            static_cast<float>((*ArrayValue)[3].get<double>()));
        return true;
    }
}
