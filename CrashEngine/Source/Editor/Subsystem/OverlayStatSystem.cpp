// 에디터 영역의 세부 동작을 구현합니다.
#include "Editor/Subsystem/OverlayStatSystem.h"

#include "Editor/EditorEngine.h"
#include "Editor/Viewport/LevelEditorViewportClient.h"
#include "Engine/Profiling/Timer.h"
#include "Engine/Profiling/MemoryStats.h"
#include "Profiling/Stats.h"
#include "Profiling/GPUProfiler.h"
#include <cstring>
#include <cstdio>

// バイト数を適切な単位 (B / KB / MB / GB) に変換して文字列化
static int FormatBytes(char* Buffer, int32 BufferSize, const char* Label, uint64 Bytes)
{
    const double B = static_cast<double>(Bytes);
    const double KB = B / 1024.0;
    const double MB = KB / 1024.0;
    const double GB = MB / 1024.0;

    if (GB >= 1.0)
        return snprintf(Buffer, BufferSize, "%s : %.2f GB", Label, GB);
    if (MB >= 1.0)
        return snprintf(Buffer, BufferSize, "%s : %.2f MB", Label, MB);
    if (KB >= 1.0)
        return snprintf(Buffer, BufferSize, "%s : %.2f KB", Label, KB);
    return snprintf(Buffer, BufferSize, "%s : %llu B", Label, static_cast<unsigned long long>(Bytes));
}

static bool UsesLightingPass(EViewMode ViewMode)
{
    switch (ViewMode)
    {
    case EViewMode::Lit_Gouraud:
    case EViewMode::Unlit:
    case EViewMode::WorldNormal:
    case EViewMode::Wireframe:
    case EViewMode::SceneDepth:
        return false;
    default:
        return true;
    }
}

static const FStatEntry* FindStatEntryByName(const TArray<FStatEntry>& Entries, const char* Name)
{
    for (const FStatEntry& Entry : Entries)
    {
        if (Entry.Name && strcmp(Entry.Name, Name) == 0)
        {
            return &Entry;
        }
    }
    return nullptr;
}

void FOverlayStatSystem::AppendLine(TArray<FOverlayStatLine>& OutLines, float Y, const FString& Text) const
{
    FOverlayStatLine Line;
    Line.Text = Text;
    Line.ScreenPosition = FVector2(Layout.StartX, Y);
    OutLines.push_back(std::move(Line));
}

void FOverlayStatSystem::RecordPickingAttempt(double ElapsedMs)
{
    LastPickingTimeMs = ElapsedMs;
    AccumulatedPickingTimeMs += ElapsedMs;
    ++PickingAttemptCount;
}

void FOverlayStatSystem::ClearDisplayFlags()
{
    bShowFPS = false;
    bShowPickingTime = false;
    bShowMemory = false;
    bShowLightCull = false;
    bShowShadow = false;
}

void FOverlayStatSystem::ShowFPS(bool bEnable)
{
    ClearDisplayFlags();
    bShowFPS = bEnable;
    FLightCullStats::SetEnabled(false);
}

void FOverlayStatSystem::ShowPickingTime(bool bEnable)
{
    ClearDisplayFlags();
    bShowPickingTime = bEnable;
    FLightCullStats::SetEnabled(false);
}

void FOverlayStatSystem::ShowMemory(bool bEnable)
{
    ClearDisplayFlags();
    bShowMemory = bEnable;
    FLightCullStats::SetEnabled(false);
}

void FOverlayStatSystem::ShowLightCull(bool bEnable)
{
    ClearDisplayFlags();
    bShowLightCull = bEnable;
    FLightCullStats::SetEnabled(bEnable);
    if (!bEnable)
    {
        FLightCullStats::ResetSample();
    }
}

void FOverlayStatSystem::ShowShadow(bool bEnable)
{
    ClearDisplayFlags();
    bShowShadow = bEnable;
    FLightCullStats::SetEnabled(false);
}

void FOverlayStatSystem::HideAll()
{
    ClearDisplayFlags();
    FLightCullStats::SetEnabled(false);
    FLightCullStats::ResetSample();
}

void FOverlayStatSystem::BuildLines(const UEditorEngine& Editor, TArray<FOverlayStatLine>& OutLines) const
{
    OutLines.clear();

    uint32 EstimatedLineCount = 0;
    if (bShowFPS)
    {
        ++EstimatedLineCount;
    }
    if (bShowPickingTime)
    {
        ++EstimatedLineCount;
    }
    if (bShowMemory)
    {
        EstimatedLineCount += 8;
    }
    if (bShowLightCull)
    {
        EstimatedLineCount += 2;
    }
    if (bShowShadow)
    {
        EstimatedLineCount += 9;
    }
    OutLines.reserve(EstimatedLineCount);

    float CurrentY = Layout.StartY;
    if (bShowFPS)
    {
        const FTimer* Timer = Editor.GetTimer();
        const float FPS = Timer ? Timer->GetDisplayFPS() : 0.0f;
        const float MS = FPS > 0.0f ? 1000.0f / FPS : 0.0f;

        char Buffer[128] = {};
        snprintf(Buffer, sizeof(Buffer), "FPS : %.1f (%.2f ms)", FPS, MS);
        CachedFPSLine = Buffer;
        AppendLine(OutLines, CurrentY, CachedFPSLine);
        CurrentY += Layout.LineHeight + Layout.GroupSpacing;
    }

    if (bShowPickingTime)
    {
        char Buffer[160] = {};
        snprintf(Buffer, sizeof(Buffer), "Picking Time %.5f ms : Num Attempts %d : Accumulated Time %.5f ms",
                 LastPickingTimeMs,
                 static_cast<int32>(PickingAttemptCount),
                 AccumulatedPickingTimeMs);
        CachedPickingLine = Buffer;
        AppendLine(OutLines, CurrentY, CachedPickingLine);
        CurrentY += Layout.LineHeight + Layout.GroupSpacing;
    }

    if (bShowMemory)
    {
        char Buffer[128] = {};

        // 할당 횟수 (단위 없음)
        snprintf(Buffer, sizeof(Buffer), "Allocation Count : %u", MemoryStats::GetTotalAllocationCount());
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        // 바이트 단위 메모리 — 자동 단위 변환 (B/KB/MB/GB)
        struct
        {
            const char* Label;
            uint64 Bytes;
        } MemEntries[] = {
            { "Total Allocated", MemoryStats::GetTotalAllocationBytes() },
            { "PixelShader Memory", MemoryStats::GetPixelShaderMemory() },
            { "VertexShader Memory", MemoryStats::GetVertexShaderMemory() },
            { "VertexBuffer Memory", MemoryStats::GetVertexBufferMemory() },
            { "IndexBuffer Memory", MemoryStats::GetIndexBufferMemory() },
            { "StaticMesh CPU Memory", MemoryStats::GetStaticMeshCPUMemory() },
            { "Texture Memory", MemoryStats::GetTextureMemory() },
        };

        for (const auto& Entry : MemEntries)
        {
            FormatBytes(Buffer, sizeof(Buffer), Entry.Label, Entry.Bytes);
            AppendLine(OutLines, CurrentY, FString(Buffer));
            CurrentY += Layout.LineHeight;
        }
    }

    if (bShowLightCull)
    {
        const FLevelEditorViewportClient* ActiveViewport = Editor.GetActiveViewport();
        const EViewMode ActiveViewMode = ActiveViewport ? ActiveViewport->GetRenderOptions().ViewMode : EViewMode::Unlit;
        if (!UsesLightingPass(ActiveViewMode))
        {
            AppendLine(OutLines, CurrentY, FString("Lighting Pass GPU Time: N/A"));
            CurrentY += Layout.LineHeight;

            AppendLine(OutLines, CurrentY, FString("Total Per-Pixel Local Light Evaluations: N/A"));
            CurrentY += Layout.LineHeight;
            return;
        }

        if (!FLightCullStats::HasSample())
        {
            AppendLine(OutLines, CurrentY, FString("Lighting Pass GPU Time: Not measured"));
            CurrentY += Layout.LineHeight;

            AppendLine(OutLines, CurrentY, FString("Total Per-Pixel Local Light Evaluations: Not measured"));
            CurrentY += Layout.LineHeight;
            return;
        }

        char Buffer[128] = {};
        snprintf(Buffer, sizeof(Buffer), "Lighting Pass GPU Time: %.3f ms", FLightCullStats::GetGPUTimeMs());
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Total Per-Pixel Local Light Evaluations: %u", FLightCullStats::GetEvaluationCount());
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;
    }

    if (bShowShadow)
    {
        const TArray<FStatEntry>& CPUStats = FStatManager::Get().GetSnapshot();
        const TArray<FStatEntry>& GPUStats = FGPUProfiler::Get().GetGPUSnapshot();

        const FStatEntry* ShadowCasterCPU = FindStatEntryByName(CPUStats, "Shadow Caster Collection");
        const FStatEntry* ShadowFrustumCPU = FindStatEntryByName(CPUStats, "Shadow Light Frustum Culling");
        const FStatEntry* ShadowAtlasCPU = FindStatEntryByName(CPUStats, "Shadow Atlas Update");
        const FStatEntry* ShadowDepthGPU = FindStatEntryByName(GPUStats, "Shadow Depth Pass");
        const FStatEntry* ShadowAtlasGPU = FindStatEntryByName(GPUStats, "Shadow Atlas Update GPU");

        const float ShadowDepthMs = ShadowDepthGPU ? static_cast<float>(ShadowDepthGPU->LastTime * 1000.0) : 0.0f;
        const float EstimatedVertexMs = ShadowDepthMs * 0.35f;
        const float EstimatedRasterMs = ShadowDepthMs * 0.40f;
        const float EstimatedDepthWriteMs = ShadowDepthMs * 0.25f;
        const double EstimatedBandwidthMB =
            static_cast<double>(FShadowPipelineStats::GetEstimatedBandwidthBytes()) / (1024.0 * 1024.0);

        char Buffer[160] = {};
        snprintf(Buffer, sizeof(Buffer), "Shadow Caster Collection CPU: %.3f ms", ShadowCasterCPU ? ShadowCasterCPU->LastTime * 1000.0 : 0.0);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Light Frustum Culling CPU: %.3f ms", ShadowFrustumCPU ? ShadowFrustumCPU->LastTime * 1000.0 : 0.0);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Shadow Depth Draw Calls: %u", FShadowPipelineStats::GetShadowDepthDrawCalls());
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Vertex Shader Cost (est.): %.3f ms", EstimatedVertexMs);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Rasterization Cost (est.): %.3f ms", EstimatedRasterMs);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Depth Write Cost (est.): %.3f ms", EstimatedDepthWriteMs);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Shadow Atlas Update CPU: %.3f ms", ShadowAtlasCPU ? ShadowAtlasCPU->LastTime * 1000.0 : 0.0);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "Shadow Atlas Update GPU: %.3f ms", ShadowAtlasGPU ? ShadowAtlasGPU->LastTime * 1000.0 : 0.0);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;

        snprintf(Buffer, sizeof(Buffer), "GPU Bandwidth (est.): %.2f MB", EstimatedBandwidthMB);
        AppendLine(OutLines, CurrentY, FString(Buffer));
        CurrentY += Layout.LineHeight;
    }
}

TArray<FOverlayStatLine> FOverlayStatSystem::BuildLines(const UEditorEngine& Editor) const
{
    TArray<FOverlayStatLine> Result;
    BuildLines(Editor, Result);
    return Result;
}
