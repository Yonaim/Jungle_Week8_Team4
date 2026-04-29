#pragma once

#include "Editor/UI/EditorPanel.h"

class FEditorControlPanel : public FEditorPanel
{
public:
    void Initialize(UEditorEngine* InEditorEngine) override;
    void Render(float DeltaTime) override;

private:
    void RenderCameraControlSection();
};
