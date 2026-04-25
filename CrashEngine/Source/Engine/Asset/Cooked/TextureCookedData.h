#pragma once

#include <filesystem>

#include "Asset/Core/TextureTypes.h"

namespace Asset
{

    struct FTextureCookedData
    {
        std::filesystem::path SourcePath;
        uint32        Width = 0;
        uint32        Height = 0;
        uint32        Channels = 0;
        bool          bSRGB = true;
        TArray<uint8> Pixels;
        EPixelFormat  Format = EPixelFormat::Unknown;

        bool IsValid() const { return Width > 0 && Height > 0 && !Pixels.empty(); }

        void Reset()
        {
            SourcePath.clear();
            Width = 0;
            Height = 0;
            Channels = 0;
            bSRGB = true;
            Pixels.clear();
            Format = EPixelFormat::Unknown;
        }
    };

} // namespace Asset
