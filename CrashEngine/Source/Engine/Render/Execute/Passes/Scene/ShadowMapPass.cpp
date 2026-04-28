#include "ShadowMapPass.h"

#include "Component/LightComponent.h"
#include "Render/Resources/Buffers/ConstantBufferData.h"
#include "Render/Resources/FrameResources.h"
#include "Render/Resources/Shadows/ShadowFilterSettings.h"
#include "Render/Scene/Proxies/Light/LightProxy.h"
#include "Render/Submission/Command/BuildDrawCommand.h"

#include <algorithm>

FShadowMapPass::~FShadowMapPass()
{
    ReleaseShadowAtlasResources();
}

bool FShadowMapPass::UpdateLightShadowAllocation(FLightProxy& Light, ID3D11Device* Device)
{
    return ShadowAllocationMap.UpdateLightShadow(Light, Device, AtlasPool);
}

void FShadowMapPass::ReleaseShadowAtlasResources()
{
    ShadowAllocationMap.Release(AtlasPool);
    AtlasPool.Release();
    MomentFilter.Release();
    RenderItems.clear();
}

ID3D11ShaderResourceView* FShadowMapPass::GetShadowAtlasSRV(uint32 PageIndex) const
{
    const FShadowAtlasPage* Page = AtlasPool.GetPage(PageIndex);
    return Page ? Page->GetDepthArraySRV() : nullptr;
}

ID3D11ShaderResourceView* FShadowMapPass::GetShadowMomentSRV(uint32 PageIndex) const
{
    const FShadowAtlasPage* Page = AtlasPool.GetPage(PageIndex);
    return Page ? Page->GetMomentArraySRV() : nullptr;
}

ID3D11ShaderResourceView* FShadowMapPass::GetShadowPreviewSRV(const FShadowMapData& ShadowMapData) const
{
    if (!ShadowMapData.bAllocated)
    {
        return nullptr;
    }

    const FShadowAtlasPage* Page = AtlasPool.GetPage(ShadowMapData.AtlasPageIndex);
    return Page ? Page->GetPreviewSliceSRV(ShadowMapData.SliceIndex) : nullptr;
}

ID3D11ShaderResourceView* FShadowMapPass::GetShadowPageSlicePreviewSRV(uint32 PageIndex, uint32 SliceIndex) const
{
    const FShadowAtlasPage* Page = AtlasPool.GetPage(PageIndex);
    return Page ? Page->GetPreviewSliceSRV(SliceIndex) : nullptr;
}

void FShadowMapPass::GetShadowPageSliceAllocations(uint32 PageIndex, uint32 SliceIndex, TArray<FShadowMapData>& OutAllocations) const
{
    OutAllocations.clear();

    const FShadowAtlasPage* Page = AtlasPool.GetPage(PageIndex);
    if (!Page)
    {
        return;
    }

    Page->GatherSliceAllocations(SliceIndex, PageIndex, OutAllocations);
}

uint32 FShadowMapPass::GetShadowAtlasPageCount() const
{
    return AtlasPool.GetPageCount();
}

void FShadowMapPass::PrepareInputs(FRenderPipelineContext& Context)
{
    ID3D11ShaderResourceView* NullSRVs[6] = {};
    Context.Context->PSSetShaderResources(0, ARRAY_SIZE(NullSRVs), NullSRVs);
}

void FShadowMapPass::PrepareTargets(FRenderPipelineContext& Context)
{
    (void)Context;
}

void FShadowMapPass::BuildDrawCommands(FRenderPipelineContext& Context)
{
    RenderItems.clear();

    auto AppendRenderItem = [&](FLightProxy* Light, const FShadowMapData* Allocation, const FMatrix& ViewProj)
    {
        if (!Light || !Allocation || !Allocation->bAllocated || RenderItems.size() >= 255)
        {
            return;
        }

        const uint16 ItemIndex = static_cast<uint16>(RenderItems.size());
        RenderItems.push_back({ Light, Allocation, ViewProj });
        for (FPrimitiveProxy* Proxy : Light->VisibleShadowCasters)
        {
            DrawCommandBuild::BuildMeshDrawCommand(*Proxy, ERenderPass::ShadowMap, Context, *Context.DrawCommandList, ItemIndex);
        }
    };

    auto& VisibleLights = Context.Submission.SceneData->Lights.VisibleLightProxies;
    for (FLightProxy* Light : VisibleLights)
    {
        if (!Light || !Light->bCastShadow)
        {
            continue;
        }

        const uint32 LightType = Light->LightProxyInfo.LightType;
        if (LightType == static_cast<uint32>(ELightType::Directional))
        {
            const FCascadeShadowMapData* CascadeShadowMapData = Light->GetCascadeShadowMapData();
            if (!CascadeShadowMapData)
            {
                continue;
            }

            const uint32 CascadeCount = std::max(1u, CascadeShadowMapData->CascadeCount);
            for (uint32 CascadeIndex = 0; CascadeIndex < CascadeCount; ++CascadeIndex)
            {
                AppendRenderItem(
                    Light,
                    &CascadeShadowMapData->Cascades[CascadeIndex],
                    CascadeShadowMapData->CascadeViewProj[CascadeIndex]);
            }
        }
        else if (LightType == static_cast<uint32>(ELightType::Spot))
        {
            const FShadowMapData* SpotShadowMapData = Light->GetSpotShadowMapData();
            if (!SpotShadowMapData)
            {
                continue;
            }

            AppendRenderItem(Light, SpotShadowMapData, Light->LightViewProj);
        }
        else if (LightType == static_cast<uint32>(ELightType::Point))
        {
            const FCubeShadowMapData* CubeShadowMapData = Light->GetCubeShadowMapData();
            if (!CubeShadowMapData)
            {
                continue;
            }

            for (uint32 FaceIndex = 0; FaceIndex < ShadowAtlas::MaxPointFaces; ++FaceIndex)
            {
                AppendRenderItem(Light, &CubeShadowMapData->Faces[FaceIndex], CubeShadowMapData->FaceViewProj[FaceIndex]);
            }
        }
    }
}

void FShadowMapPass::BuildDrawCommands(FRenderPipelineContext& Context, const FPrimitiveProxy& Proxy)
{
    (void)Context;
    (void)Proxy;
}

void FShadowMapPass::SubmitDrawCommands(FRenderPipelineContext& Context)
{
    if (!Context.DrawCommandList || RenderItems.empty())
    {
        return;
    }

    uint32 GlobalStart = 0;
    uint32 GlobalEnd = 0;
    Context.DrawCommandList->GetPassRange(ERenderPass::ShadowMap, GlobalStart, GlobalEnd);
    if (GlobalStart >= GlobalEnd)
    {
        return;
    }

    ID3D11RenderTargetView* SavedRTV = nullptr;
    ID3D11DepthStencilView* SavedDSV = nullptr;
    Context.Context->OMGetRenderTargets(1, &SavedRTV, &SavedDSV);

    D3D11_VIEWPORT SavedViewport = {};
    uint32 NumViewports = 1;
    Context.Context->RSGetViewports(&NumViewports, &SavedViewport);

    D3D11_RECT SavedScissor = {};
    uint32 NumScissors = 1;
    Context.Context->RSGetScissorRects(&NumScissors, &SavedScissor);

    TArray<FDrawCommand>& Commands = Context.DrawCommandList->GetCommands();
    bool ClearedSlices[ShadowAtlas::MaxPages][ShadowAtlas::SliceCount] = {};

    uint32 CurrentIdx = GlobalStart;
    while (CurrentIdx < GlobalEnd)
    {
        const uint8 ItemIndex = static_cast<uint8>((Commands[CurrentIdx].SortKey >> 52) & 0xFF);
        const uint32 RangeStart = CurrentIdx;
        while (CurrentIdx < GlobalEnd && static_cast<uint8>((Commands[CurrentIdx].SortKey >> 52) & 0xFF) == ItemIndex)
        {
            ++CurrentIdx;
        }
        const uint32 RangeEnd = CurrentIdx;

        if (ItemIndex >= RenderItems.size())
        {
            continue;
        }

        const FShadowRenderItem& Item = RenderItems[ItemIndex];
        if (!Item.Allocation || !Item.Allocation->bAllocated)
        {
            continue;
        }

        FShadowAtlasPage* AtlasPage = AtlasPool.GetPage(Item.Allocation->AtlasPageIndex);
        if (!AtlasPage)
        {
            continue;
        }

        const bool bUsesVSMMoments = GetShadowFilterMethod() == EShadowFilterMethod::VSM;
        if (bUsesVSMMoments && !AtlasPage->EnsureMomentResources(Context.Device->GetDevice()))
        {
            continue;
        }

        ID3D11DepthStencilView* DSV = AtlasPage->GetSliceDSV(Item.Allocation->SliceIndex);
        ID3D11RenderTargetView* RTV = AtlasPage->GetMomentSliceRTV(Item.Allocation->SliceIndex);
        if (!DSV)
        {
            continue;
        }

        if (!ClearedSlices[Item.Allocation->AtlasPageIndex][Item.Allocation->SliceIndex])
        {
            Context.Context->ClearDepthStencilView(DSV, D3D11_CLEAR_DEPTH, 0.0f, 0);
            if (RTV)
            {
                const float ClearMomentColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                Context.Context->ClearRenderTargetView(RTV, ClearMomentColor);
            }
            ClearedSlices[Item.Allocation->AtlasPageIndex][Item.Allocation->SliceIndex] = true;
        }

        D3D11_VIEWPORT ShadowViewport = {};
        ShadowViewport.TopLeftX = static_cast<float>(Item.Allocation->ViewportRect.X);
        ShadowViewport.TopLeftY = static_cast<float>(Item.Allocation->ViewportRect.Y);
        ShadowViewport.Width = static_cast<float>(Item.Allocation->ViewportRect.Width);
        ShadowViewport.Height = static_cast<float>(Item.Allocation->ViewportRect.Height);
        ShadowViewport.MinDepth = 0.0f;
        ShadowViewport.MaxDepth = 1.0f;
        Context.Context->RSSetViewports(1, &ShadowViewport);

        D3D11_RECT ScissorRect = {};
        ScissorRect.left = static_cast<LONG>(Item.Allocation->ViewportRect.X);
        ScissorRect.top = static_cast<LONG>(Item.Allocation->ViewportRect.Y);
        ScissorRect.right = static_cast<LONG>(Item.Allocation->ViewportRect.X + Item.Allocation->ViewportRect.Width);
        ScissorRect.bottom = static_cast<LONG>(Item.Allocation->ViewportRect.Y + Item.Allocation->ViewportRect.Height);
        Context.Context->RSSetScissorRects(1, &ScissorRect);

        Context.Context->OMSetRenderTargets(1, &RTV, DSV);

        FFrameCBData ShadowFrameData = {};
        ShadowFrameData.View = FMatrix::Identity;
        ShadowFrameData.Projection = Item.ViewProj;
        ShadowFrameData.InvViewProj = Item.ViewProj.GetInverse();
        Context.Resources->FrameBuffer.Update(Context.Context, &ShadowFrameData, sizeof(FFrameCBData));

        Context.DrawCommandList->SubmitRange(RangeStart, RangeEnd, *Context.Device, Context.Context, *Context.StateCache);
    }

    for (uint32 PageIndex = 0; PageIndex < AtlasPool.GetPageCount(); ++PageIndex)
    {
        FShadowAtlasPage* AtlasPage = AtlasPool.GetPage(PageIndex);
        if (!AtlasPage)
        {
            continue;
        }

        for (uint32 SliceIndex = 0; SliceIndex < ShadowAtlas::SliceCount; ++SliceIndex)
        {
            if (!ClearedSlices[PageIndex][SliceIndex])
            {
                continue;
            }

            MomentFilter.BlurMomentTextureSlice(Context, *AtlasPage, SliceIndex);
        }

        if (ID3D11ShaderResourceView* MomentSRV = AtlasPage->GetMomentArraySRV())
        {
            ID3D11RenderTargetView* NullRTV = nullptr;
            Context.Context->OMSetRenderTargets(1, &NullRTV, nullptr);
            Context.Context->GenerateMips(MomentSRV);
        }
    }

    Context.Context->RSSetViewports(1, &SavedViewport);
    Context.Context->RSSetScissorRects(1, &SavedScissor);
    Context.Context->OMSetRenderTargets(1, &SavedRTV, SavedDSV);
    if (SavedRTV) SavedRTV->Release();
    if (SavedDSV) SavedDSV->Release();

    if (Context.SceneView)
    {
        FFrameCBData MainFrameData = {};
        MainFrameData.View = Context.SceneView->View;
        MainFrameData.Projection = Context.SceneView->Proj;
        MainFrameData.InvViewProj = (Context.SceneView->View * Context.SceneView->Proj).GetInverse();
        MainFrameData.CameraWorldPos = Context.SceneView->CameraPosition;
        MainFrameData.bIsWireframe = (Context.SceneView->ViewMode == EViewMode::Wireframe);
        MainFrameData.WireframeColor = Context.SceneView->WireframeColor;
        Context.Resources->FrameBuffer.Update(Context.Context, &MainFrameData, sizeof(FFrameCBData));
    }
}
