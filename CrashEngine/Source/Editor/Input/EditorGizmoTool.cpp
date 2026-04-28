#include "EditorGizmoTool.h"

#include "Collision/RayUtils.h"
#include "Component/CameraComponent.h"
#include "Component/GizmoComponent.h"
#include "Editor/EditorEngine.h"
#include "Editor/Input/EditorViewportInputController.h"
#include "Editor/Selection/SelectionManager.h"
#include "Editor/Viewport/EditorViewportClient.h"
#include "GameFramework/World.h"

bool FEditorGizmoTool::HandleInput(float DeltaTime)
{
    (void)DeltaTime;

    if (!Owner || !Controller || !Owner->GetCamera() || !Owner->GetGizmo() || !Owner->GetWorld())
    {
        return false;
    }

    UCameraComponent* Camera = Owner->GetCamera();
    UGizmoComponent* Gizmo = Owner->GetGizmo();

	if (Owner->GetWorld()->GetWorldType() == EWorldType::PIE)
    {
        if (Gizmo->IsHolding())
        {
            Gizmo->DragEnd();
        }
        else if (Gizmo->IsPressedOnHandle())
        {
            Gizmo->SetPressedOnHandle(false);
        }

        return false;
    }

    Gizmo->ApplyScreenSpaceScaling(Camera->GetWorldLocation(), Camera->IsOrthogonal(), Camera->GetOrthoWidth());

    Gizmo->SetAxisMask(UGizmoComponent::ComputeAxisMask(Owner->GetRenderOptions().ViewportType, Gizmo->GetMode()));

    FRay Ray;
    if (!BuildMouseRay(Ray))
    {
        return Gizmo->IsHolding() || Gizmo->IsPressedOnHandle();
    }

    const FEditorViewportFrameInput& Input = Controller->GetCurrentInput();
    const bool bMouseMoved = Input.MouseAxisDelta.x != 0 || Input.MouseAxisDelta.y != 0;
    const bool bLeftDragging = Input.bLeftDown && bMouseMoved;

    if (!Gizmo->IsHolding())
    {
        FHitResult HoverHit{};
        if (!FRayUtils::RaycastComponent(Gizmo, Ray, HoverHit))
        {
            Gizmo->UpdateHoveredAxis(-1);
        }
    }

    if (Input.bLeftReleased)
    {
        if (Gizmo->IsHolding() && bMouseMoved)
        {
            Gizmo->UpdateDrag(Ray);
        }

        return HandleDragEnd();
    }

    if (Input.bLeftPressed)
    {
        return HandleDragStart(Ray);
    }

    if (bLeftDragging)
    {
        return HandleDragUpdate(Ray);
    }

    return Gizmo->IsHolding() || Gizmo->IsPressedOnHandle();
}

void FEditorGizmoTool::ResetState()
{
    UGizmoComponent* Gizmo = Owner ? Owner->GetGizmo() : nullptr;
    if (!Gizmo)
    {
        return;
    }

    if (Gizmo->IsHolding())
    {
        Gizmo->DragEnd();
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EditorEngine->GetUndoManager().CancelTransform();
        }
    }
    else if (Gizmo->IsPressedOnHandle())
    {
        Gizmo->SetPressedOnHandle(false);
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EditorEngine->GetUndoManager().CancelTransform();
        }
    }
}

bool FEditorGizmoTool::HandleDragStart(const FRay& Ray)
{
    UGizmoComponent* Gizmo = Owner ? Owner->GetGizmo() : nullptr;
    if (!Gizmo)
    {
        return false;
    }

    FHitResult HitResult{};

    if (FRayUtils::RaycastComponent(Gizmo, Ray, HitResult))
    {
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            if (FSelectionManager* SelectionManager = Owner->GetSelectionManager())
            {
                EditorEngine->GetUndoManager().BeginTransform(Owner->GetWorld(), SelectionManager->GetSelectedActors());
            }
        }

        Gizmo->SetPressedOnHandle(true);
        return true;
    }

	Gizmo->SetPressedOnHandle(false);
    return false;
}

bool FEditorGizmoTool::HandleDragUpdate(const FRay& Ray)
{
    UGizmoComponent* Gizmo = Owner ? Owner->GetGizmo() : nullptr;
    if (!Gizmo)
    {
        return false;
    }

    if (Gizmo->IsPressedOnHandle() && !Gizmo->IsHolding())
    {
        Gizmo->SetHolding(true);
    }

    if (Gizmo->IsHolding())
    {
        Gizmo->UpdateDrag(Ray);
        return true;
    }

    return false;
}

bool FEditorGizmoTool::HandleDragEnd()
{
    UGizmoComponent* Gizmo = Owner ? Owner->GetGizmo() : nullptr;
    if (!Gizmo)
    {
        return false;
    }

    if (Gizmo->IsHolding())
    {
        Gizmo->DragEnd();
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EditorEngine->GetUndoManager().EndTransform();
        }
        return true;
    }

    if (Gizmo->IsPressedOnHandle())
    {
        Gizmo->SetPressedOnHandle(false);
        if (UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine))
        {
            EditorEngine->GetUndoManager().EndTransform();
        }
        return true;
    }

    return false;
}
