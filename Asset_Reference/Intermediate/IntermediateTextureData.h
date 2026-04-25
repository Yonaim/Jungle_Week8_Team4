#pragma once

#include "Asset/Core/TextureTypes.h"

namespace Asset
{
    struct FIntermediateTextureData
    {
        uint32        Width = 0;
        uint32        Height = 0;
        uint32        Channels = 0;
        EPixelFormat  Format = EPixelFormat::Unknown;
        TArray<uint8> Pixels;
    };

} // namespace Asset
