#pragma once

#include "Asset/Core/SubUVAtlasTypes.h"

namespace Asset
{

    struct FIntermediateSubUVAtlasData
    {
        FSubUVAtlasInfo               Info;
        TArray<FSubUVFrame>           Frames;
        TMap<FString, FSubUVSequence> Sequences;
        FWString                      AtlasImagePath;
    };

} // namespace Asset
