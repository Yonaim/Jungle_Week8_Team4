#pragma once

#include "Core/CoreMinimal.h"

namespace Asset
{

    class FSourceHash
    {
      public:
        static bool Compute(const TArray<uint8>& Bytes, FString& OutHash);
    };

} // namespace Asset
