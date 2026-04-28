#pragma once

#include "Render/Submission/Atlas/ShadowAtlasPage.h"

class FLightProxy;

// Light별 atlas allocation record를 보관하고 재할당 정책을 적용합니다.
struct FLightShadowRecord
{
    uint32                Resolution           = 0;
    uint32                CascadeCount         = 0;
    uint32                LightType            = 0;
    FCascadeShadowMapData CascadeShadowMapData = {};
    FShadowMapData        SpotShadowMapData    = {};
    FCubeShadowMapData    CubeShadowMapData    = {};
};

// 실제 역할은 light -> atlas allocation record 매핑 테이블에 가깝습니다.
class FShadowAtlasAllocationMap
{
public:
    void Release(FShadowAtlasPool& AtlasPool);
    void RemoveLight(FLightProxy* Light, FShadowAtlasPool& AtlasPool);
    bool UpdateLightShadow(FLightProxy& Light, ID3D11Device* Device, FShadowAtlasPool& AtlasPool);

private:
    void FreeRecord(FLightShadowRecord& Record, FShadowAtlasPool& AtlasPool);
    bool AllocateDirectional(FLightShadowRecord& Record, FLightProxy& Light, ID3D11Device* Device, FShadowAtlasPool& AtlasPool);
    bool AllocateSpot(FLightShadowRecord& Record, FLightProxy& Light, ID3D11Device* Device, FShadowAtlasPool& AtlasPool);
    bool AllocatePoint(FLightShadowRecord& Record, FLightProxy& Light, ID3D11Device* Device, FShadowAtlasPool& AtlasPool);

private:
    TMap<FLightProxy*, FLightShadowRecord> Records;
};
