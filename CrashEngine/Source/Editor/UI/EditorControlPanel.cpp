#include "Editor/UI/EditorControlPanel.h"

#include "Editor/EditorEngine.h"
#include "Editor/Settings/EditorSettings.h"
#include "Editor/Viewport/LevelEditorViewportClient.h"
#include "Component/CameraComponent.h"
#include "ImGui/imgui.h"

void FEditorControlPanel::Initialize(UEditorEngine* InEditorEngine)
{
    FEditorPanel::Initialize(InEditorEngine);
}

void FEditorControlPanel::Render(float DeltaTime)
{
    (void)DeltaTime;
    if (!EditorEngine)
    {
        return;
    }

    ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(420.0f, 260.0f), ImGuiCond_Once);
    ImGui::Begin("Camera Control");
    RenderCameraControlSection();
    ImGui::End();
}

void FEditorControlPanel::RenderCameraControlSection()
{
    FLevelEditorViewportClient* ActiveViewport = EditorEngine ? EditorEngine->GetActiveViewport() : nullptr;
    UCameraComponent* Camera = ActiveViewport ? ActiveViewport->GetCamera() : EditorEngine->GetCamera();
    if (!Camera || !ActiveViewport)
    {
        ImGui::TextDisabled("No active viewport camera.");
        return;
    }

    const bool bIsPilotingActor = ActiveViewport->IsPilotingActor();
    if (bIsPilotingActor)
    {
        ImGui::TextDisabled("Pilot mode active. Showing saved editor camera values.");
    }

    float CameraFOV_Deg = Camera->GetFOV() * RAD_TO_DEG;
    if (ImGui::DragFloat("Camera FOV", &CameraFOV_Deg, 0.5f, 1.0f, 90.0f))
    {
        Camera->SetFOV(CameraFOV_Deg * DEG_TO_RAD);
    }

    float CameraSpeed = FEditorSettings::Get().CameraSpeed;
    if (ImGui::DragFloat("Camera Speed", &CameraSpeed, 0.1f, 0.1f, 200.0f, "%.2f"))
    {
        FEditorSettings::Get().CameraSpeed = Clamp(CameraSpeed, 0.1f, 200.0f);
    }

    float OrthoWidth = Camera->GetOrthoWidth();
    if (ImGui::DragFloat("Ortho Width", &OrthoWidth, 0.1f, 0.1f, 1000.0f))
    {
        Camera->SetOrthoWidth(Clamp(OrthoWidth, 0.1f, 1000.0f));
    }

    const FVector DisplayCamPos = ActiveViewport->GetEditorCameraLocationForUI();
    float CameraLocation[3] = { DisplayCamPos.X, DisplayCamPos.Y, DisplayCamPos.Z };
    ImGui::BeginDisabled(bIsPilotingActor);
    if (ImGui::DragFloat3("Camera Location", CameraLocation, 0.1f))
    {
        Camera->SetWorldLocation(FVector(CameraLocation[0], CameraLocation[1], CameraLocation[2]));
    }

    const FRotator DisplayCamRot = ActiveViewport->GetEditorCameraRotationForUI();
    float CameraRotation[3] = { DisplayCamRot.Roll, DisplayCamRot.Pitch, DisplayCamRot.Yaw };
    if (ImGui::DragFloat3("Camera Rotation", CameraRotation, 0.1f))
    {
        Camera->SetRelativeRotation(FRotator(CameraRotation[1], CameraRotation[2], CameraRotation[0]));
    }
    ImGui::EndDisabled();
}
