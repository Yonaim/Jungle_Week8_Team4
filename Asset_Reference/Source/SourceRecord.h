#pragma once

#include <filesystem>

#include "Core/CoreMinimal.h"

namespace Asset
{

    struct FSourceRecord
    {
        std::filesystem::path NormalizedPath;
        uint64                FileSize = 0;
        uint64                LastWriteTimeTicks = 0;
        FString               ContentHash;
        bool                  bHasContentHash = false;
    };

} // namespace Asset
