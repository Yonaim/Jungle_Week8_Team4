#pragma once

#include "Core/CoreTypes.h"

enum class EShadowMapMethod : uint32
{
    Standard = 0,
    PSM = 1,
    Cascade = 2,
};

inline EShadowMapMethod GShadowMapMethod = EShadowMapMethod::Standard;
inline bool GEnableMobilityAwareShadowCaching = false;

inline EShadowMapMethod GetShadowMapMethod()
{
    return GShadowMapMethod;
}

inline void SetShadowMapMethod(EShadowMapMethod InMethod)
{
    GShadowMapMethod = InMethod;
}

inline bool IsMobilityAwareShadowCachingEnabled()
{
    return GEnableMobilityAwareShadowCaching;
}

inline void SetMobilityAwareShadowCachingEnabled(bool bEnabled)
{
    GEnableMobilityAwareShadowCaching = bEnabled;
}

inline const char* GetShadowMapMethodName(EShadowMapMethod InMethod)
{
    switch (InMethod)
    {
    case EShadowMapMethod::Standard:
        return "Standard";
    case EShadowMapMethod::PSM:
        return "PSM (Perspective Shadow Map)";
    case EShadowMapMethod::Cascade:
        return "Cascade";
    default:
        return "Unknown";
    }
}
