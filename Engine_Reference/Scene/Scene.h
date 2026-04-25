#pragma once

#include "Core/Containers/Array.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/Types/SceneShowFlags.h"
#include "CameraInfo.h"

namespace Engine::Component { class ULineBatchComponent; }
class AActor;

class ENGINE_API FScene
{
  public:
    FScene();
    ~FScene();

    void                   AddActor(AActor* InActor) { Actors.push_back(InActor); }
    bool                   RemoveActor(AActor* InActor);
    const TArray<AActor*>& GetActors() const { return Actors; }

    void Tick(float DeltaTime);

    void BuildRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const;
    void Clear();

    TArray<AActor*>* GetActors() { return &Actors; }
    
    void SetEditorCameraInfo(const FCameraInfo& Info) { CameraInfo = Info; }
    const FCameraInfo& GetEditorCameraInfo() const { return CameraInfo; }

  private:
    TArray<AActor*> Actors;

    // 4주차 용 레거시 씬 형식에서 카메라 정보를 저장하기 위한 임시 필드입니다. 개선된 씬 형식이 도입되면 제거해주세요.
    FCameraInfo CameraInfo;
};
