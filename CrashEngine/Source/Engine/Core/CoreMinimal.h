#pragma once

#include "Core/CoreTypes.h"
#include "Core/EngineAPI.h"
#include "Core/EngineTypes.h"
#include "Math/Vector.h"

#include <cstdio>
#include <string>

using FWString = std::wstring;

enum class ELogLevel : uint8
{
    Verbose,
    Debug,
    Warning,
    Error,
};

inline constexpr const char* AssetSource = "AssetSource";
inline constexpr const char* AssetCache = "AssetCache";

#ifndef UE_LOG
#define UE_LOG(Category, Level, Format, ...)                                                \
    do                                                                                      \
    {                                                                                       \
        (void)(Category);                                                                   \
        (void)(Level);                                                                      \
        std::printf(Format "\n", ##__VA_ARGS__);                                            \
    } while (0)
#endif
