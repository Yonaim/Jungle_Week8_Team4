#ifndef LIGHTING_COMMON_HLSLI
#define LIGHTING_COMMON_HLSLI

#include "CommonTypes.hlsli"

float3 GetAmbientColor()
{
    return Ambient.Color * Ambient.Intensity;
}

float4 ComputeGouraudLighting(float4 BaseColor, float4 GouraudL)
{
    return float4(BaseColor.rgb * GouraudL.rgb, BaseColor.a);
}

float3 ComputeGouraudLightingFactor(float3 Normal)
{
    return float3(0, 0, 0); // 암전 테스트
}

float4 ComputeLambertLighting(float4 BaseColor, float3 Normal)
{
    return float4(0, 0, 0, BaseColor.a); // 암전 테스트
}

float4 ComputeBlinnPhongLighting(float4 BaseColor, float3 Normal, float4 MaterialParam, float2 UV)
{
    return float4(0, 0, 0, BaseColor.a); // 암전 테스트
}
#endif