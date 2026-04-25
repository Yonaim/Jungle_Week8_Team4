#pragma once

#include <memory>

#include "Core/Containers/Array.h"
#include "Core/CoreMinimal.h"
#include "Renderer/SceneRenderData.h"
#include "Renderer/Types/SceneShowFlags.h"

class FScene;

class ENGINE_API FWorld
{
  public:
    FWorld();
    ~FWorld() = default;

    FWorld(const FWorld&) = delete;
    FWorld& operator=(const FWorld&) = delete;

    FWorld(FWorld&&) = default;
    FWorld& operator=(FWorld&&) = default;

    FScene*                                GetActiveScene() const { return ActiveScene; }
    FScene*                                GetPersistentScene() const { return PersistentScene; }
    const TArray<std::unique_ptr<FScene>>& GetScenes() const { return Scenes; }

    FScene* AddScene(std::unique_ptr<FScene> InScene, bool bSetActive = false);
    FScene* CreateScene(bool bSetActive = false);
    void    SetActiveScene(FScene* InScene);
    bool    ReplaceActiveScene(std::unique_ptr<FScene> InScene);

    void Tick(float DeltaTime);
    void BuildRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const;
    void Clear();

  private:
    void EnsurePersistentScene();
    void CreatePersistentSceneActor();

  private:
    TArray<std::unique_ptr<FScene>> Scenes;
    FScene*                         ActiveScene = nullptr;
    FScene*                         PersistentScene = nullptr;
};
