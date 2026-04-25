#pragma once

#include "Asset/Core/AssetCommonTypes.h"

namespace Asset
{

    struct FFontGlyph
    {
        uint32 Id = 0;
        uint32 X = 0;
        uint32 Y = 0;
        uint32 Width = 0;
        uint32 Height = 0;
        int32  XOffset = 0;
        int32  YOffset = 0;
        int32  XAdvance = 0;
        uint32 Page = 0;
        uint32 Channel = 0;

        bool IsValid() const { return Width > 0 && Height > 0; }
    };

    struct FFontInfo
    {
        FString Face;
        int32   Size = 0;
        bool    bBold = false;
        bool    bItalic = false;
        bool    bUnicode = false;
    };

    struct FFontCommon
    {
        uint32 LineHeight = 0;
        uint32 Base = 0;
        uint32 ScaleW = 0;
        uint32 ScaleH = 0;
        uint32 Pages = 0;
        bool   bPacked = false;
    };

} // namespace Asset
