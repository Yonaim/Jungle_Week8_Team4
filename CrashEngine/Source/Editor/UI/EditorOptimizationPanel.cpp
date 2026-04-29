#include "Editor/UI/EditorOptimizationPanel.h"

#include "Editor/EditorEngine.h"
#include "Editor/Settings/EditorSettings.h"
#include "Editor/Viewport/LevelEditorViewportClient.h"
#include "ImGui/imgui.h"
#include "Render/Resources/Shadows/ShadowMapSettings.h"

void FEditorOptimizationPanel::Render(float DeltaTime)
{
    (void)DeltaTime;
    if (!EditorEngine)
    {
        return;
    }

    ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(420.0f, 220.0f), ImGuiCond_Once);
    if (!ImGui::Begin("Optimization Option"))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Shadow Caching");
    bool bMobilityAwareShadowCaching = FEditorSettings::Get().bMobilityAwareShadowCaching;
    if (ImGui::Checkbox("Mobility-aware Shadow Caching", &bMobilityAwareShadowCaching))
    {
        FEditorSettings::Get().bMobilityAwareShadowCaching = bMobilityAwareShadowCaching;
        SetMobilityAwareShadowCachingEnabled(bMobilityAwareShadowCaching);
    }
    ImGui::TextDisabled("Off: redraw every frame. On: reuse static/stationary shadow data when possible.");

    ImGui::SeparatorText("Light Culling Options");
    FLevelEditorViewportClient* ActiveViewport = EditorEngine->GetActiveViewport();
    if (!ActiveViewport)
    {
        ImGui::TextDisabled("No active viewport.");
        ImGui::End();
        return;
    }

    FViewportRenderOptions& Opts = ActiveViewport->GetRenderOptions();
    int Mode = Opts.ShowFlags.b25DCulling ? 1 : 0;
    if (ImGui::RadioButton("2D Tile Culling", Mode == 0))
    {
        Mode = 0;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("2.5D Tile Culling", Mode == 1))
    {
        Mode = 1;
    }
    Opts.ShowFlags.b25DCulling = (Mode == 1);

    ImGui::End();
}
