#pragma once

#include <filesystem>

#include "Core/CoreMinimal.h"

namespace Asset
{

    class FSourceLoader
    {
      public:
        static bool QueryFileInfo(const std::filesystem::path& Path, uint64& OutFileSize,
                                  uint64& OutWriteTimeTicks);
        static bool ReadAllBytes(const std::filesystem::path& Path, TArray<uint8>& OutBytes);
    };

} // namespace Asset
