#pragma once

#include "Render/Execute/Passes/Base/MeshPassBase.h"
#include "Render/Submission/Atlas/LightShadowAllocationRegistry.h"
#include "Render/Submission/Atlas/ShadowMomentFilter.h"

class FLightProxy;

class FShadowMapPass : public FMeshPassBase
{
public:
    ~FShadowMapPass() override;

    void PrepareInputs(FRenderPipelineContext& Context) override;
    void PrepareTargets(FRenderPipelineContext& Context) override;
    void BuildDrawCommands(FRenderPipelineContext& Context) override;
    void BuildDrawCommands(FRenderPipelineContext& Context, const FPrimitiveProxy& Proxy) override;
    void SubmitDrawCommands(FRenderPipelineContext& Context) override;

    bool UpdateLightShadowAllocation(FLightProxy& Light, ID3D11Device* Device);
    void ReleaseShadowAtlasResources();

    ID3D11ShaderResourceView* GetShadowAtlasSRV(uint32 PageIndex) const;
    ID3D11ShaderResourceView* GetShadowMomentSRV(uint32 PageIndex) const;
    ID3D11ShaderResourceView* GetShadowPreviewSRV(const FShadowMapData& ShadowMapData) const;
    ID3D11ShaderResourceView* GetShadowPageSlicePreviewSRV(uint32 PageIndex, uint32 SliceIndex) const;
    void                      GetShadowPageSliceAllocations(uint32 PageIndex, uint32 SliceIndex, TArray<FShadowMapData>& OutAllocations) const;
    uint32                    GetShadowAtlasPageCount() const;
    uint32                    GetShadowAtlasSize() const { return ShadowAtlas::AtlasSize; }

private:
    struct FShadowRenderItem
    {
        FLightProxy*          Light      = nullptr;
        const FShadowMapData* Allocation = nullptr;
        FMatrix               ViewProj   = FMatrix::Identity;
    };

private:
    FShadowAtlasPool               AtlasPool;
    FLightShadowAllocationRegistry ShadowAllocationMap;
    FShadowMomentFilter            MomentFilter;
    TArray<FShadowRenderItem>      RenderItems;
};
