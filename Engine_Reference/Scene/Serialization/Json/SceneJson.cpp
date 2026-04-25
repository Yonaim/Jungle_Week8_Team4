#include "Engine/Scene/Serialization/Json/SceneJson.h"

bool FSceneJsonParser::Parse(const FString& Source, FSceneJsonValue& OutValue,
                             FString* OutErrorMessage)
{
    try
    {
        OutValue = FSceneJsonValue::parse(Source);
        return true;
    }
    catch (const nlohmann::json::exception& Exception)
    {
        if (OutErrorMessage != nullptr)
        {
            *OutErrorMessage = FString("Failed to parse scene JSON: ") + Exception.what();
        }
        return false;
    }
}

FString FSceneJsonWriter::Write(const FSceneJsonValue& Value, bool bPretty)
{
    return bPretty ? Value.dump(4) : Value.dump();
}
