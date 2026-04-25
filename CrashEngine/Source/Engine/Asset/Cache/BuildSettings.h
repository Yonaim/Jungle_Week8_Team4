#pragma once

#include <sstream>

#include "Core/CoreMinimal.h"

namespace Asset
{

    struct FTextureBuildSettings
    {
        bool bSRGB = true;
        bool bGenerateMips = false;

        FString ToKeyString() const
        {
            std::ostringstream Oss;
            Oss << "SRGB=" << (bSRGB ? 1 : 0) << ";Mips=" << (bGenerateMips ? 1 : 0);
            return Oss.str();
        }
    };

    struct FStaticMeshBuildSettings
    {
        bool bRecomputeNormals = false;

        // OBJ source UV is treated as bottom-left origin.
        // Engine cooked UV convention is top-left origin.
        // Flip V during cook by default.
        bool bFlipV = true;

        FString ToKeyString() const
        {
            std::ostringstream Oss;
            Oss << "RecomputeNormals=" << (bRecomputeNormals ? 1 : 0)
                << ";FlipV=" << (bFlipV ? 1 : 0);
            return Oss.str();
        }
    };

} // namespace Asset
