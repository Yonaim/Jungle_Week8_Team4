#pragma once

#include "Asset/Core/AssetCommonTypes.h"

namespace Asset
{

    struct FSubUVAtlasInfo
    {
        FString Name;
        uint32  FrameWidth = 0;
        uint32  FrameHeight = 0;
        uint32  Columns = 1;
        uint32  Rows = 1;
        uint32  FrameCount = 0;
        float   FPS = 0.0f;
        bool    bLoop = true;
    };

    struct FSubUVFrame
    {
        uint32 Id = 0;
        uint32 X = 0;
        uint32 Y = 0;
        uint32 Width = 0;
        uint32 Height = 0;
        float  PivotX = 0.5f;
        float  PivotY = 0.5f;
        float  Duration = 0.0f;

        bool IsValid() const { return Width > 0 && Height > 0; }
    };

    struct FSubUVSequence
    {
        FString Name;
        uint32  StartFrame = 0;
        uint32  EndFrame = 0;
        bool    bLoop = true;
    };

} // namespace Asset
