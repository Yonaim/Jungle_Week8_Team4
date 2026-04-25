#include "Engine/Scene/World.h"

#include <algorithm>

#include "Engine/Component/Mesh/LineBatchComponent.h"
#include "Engine/Game/Actor.h"
#include "Engine/Scene/Scene.h"

FWorld::FWorld()
{
    std::unique_ptr<FScene> InitialScene = std::make_unique<FScene>();
    ActiveScene = InitialScene.get();
    Scenes.push_back(std::move(InitialScene));
    EnsurePersistentScene();
}

FScene* FWorld::AddScene(std::unique_ptr<FScene> InScene, bool bSetActive)
{
    if (!InScene)
    {
        return nullptr;
    }

    FScene* RawScene = InScene.get();
    Scenes.push_back(std::move(InScene));

    if (ActiveScene == nullptr || bSetActive)
    {
        ActiveScene = RawScene;
    }

    return RawScene;
}

FScene* FWorld::CreateScene(bool bSetActive)
{
    return AddScene(std::make_unique<FScene>(), bSetActive);
}

void FWorld::SetActiveScene(FScene* InScene)
{
    if (InScene == nullptr || InScene == PersistentScene)
    {
        return;
    }

    for (const std::unique_ptr<FScene>& Scene : Scenes)
    {
        if (Scene.get() == InScene)
        {
            ActiveScene = InScene;
            return;
        }
    }
}

bool FWorld::ReplaceActiveScene(std::unique_ptr<FScene> InScene)
{
    if (!InScene)
    {
        return false;
    }

    EnsurePersistentScene();

    if (ActiveScene == nullptr || ActiveScene == PersistentScene)
    {
        FScene* NewActiveScene = InScene.get();
        Scenes.insert(Scenes.begin(), std::move(InScene));
        ActiveScene = NewActiveScene;
        return true;
    }

    for (std::unique_ptr<FScene>& Scene : Scenes)
    {
        if (Scene.get() == ActiveScene)
        {
            Scene = std::move(InScene);
            ActiveScene = Scene.get();
            return true;
        }
    }

    FScene* NewActiveScene = InScene.get();
    Scenes.insert(Scenes.begin(), std::move(InScene));
    ActiveScene = NewActiveScene;
    return true;
}

void FWorld::Tick(float DeltaTime)
{
    for (const std::unique_ptr<FScene>& Scene : Scenes)
    {
        if (Scene)
        {
            Scene->Tick(DeltaTime);
        }
    }
}

void FWorld::BuildRenderData(FSceneRenderData& OutRenderData, ESceneShowFlags InShowFlags) const
{
    const FSceneView* SceneView = OutRenderData.SceneView;
    const EViewModeIndex ViewMode = OutRenderData.ViewMode;
    const bool bUseInstancing = OutRenderData.bUseInstancing;

    auto AppendScene = [&](const FScene* Scene)
    {
        if (Scene == nullptr)
        {
            return;
        }

        FSceneRenderData SceneRenderData;
        SceneRenderData.SceneView = SceneView;
        SceneRenderData.ViewMode = ViewMode;
        SceneRenderData.bUseInstancing = bUseInstancing;
        Scene->BuildRenderData(SceneRenderData, InShowFlags);

        OutRenderData.RenderCommands.insert(OutRenderData.RenderCommands.end(),
                                    SceneRenderData.RenderCommands.begin(),
                                    SceneRenderData.RenderCommands.end());
    };

    AppendScene(ActiveScene);
    if (PersistentScene != nullptr && PersistentScene != ActiveScene)
    {
        AppendScene(PersistentScene);
    }

    OutRenderData.SceneView = SceneView;
    OutRenderData.ViewMode = ViewMode;
    OutRenderData.bUseInstancing = bUseInstancing;
}

void FWorld::Clear()
{
    Scenes.clear();
    ActiveScene = nullptr;
    PersistentScene = nullptr;

    std::unique_ptr<FScene> InitialScene = std::make_unique<FScene>();
    ActiveScene = InitialScene.get();
    Scenes.push_back(std::move(InitialScene));
    EnsurePersistentScene();
}

void FWorld::EnsurePersistentScene()
{
    if (PersistentScene != nullptr)
    {
        return;
    }

    std::unique_ptr<FScene> PersistentSceneStorage = std::make_unique<FScene>();
    PersistentScene = PersistentSceneStorage.get();
    Scenes.push_back(std::move(PersistentSceneStorage));
    CreatePersistentSceneActor();
}

void FWorld::CreatePersistentSceneActor()
{
    if (PersistentScene == nullptr)
    {
        return;
    }

    const TArray<AActor*>* Actors = PersistentScene->GetActors();
    if (Actors == nullptr)
    {
        return;
    }

    for (AActor* Actor : *Actors)
    {
        if (Actor == nullptr)
        {
            continue;
        }

        for (Engine::Component::USceneComponent* Component : Actor->GetOwnedComponents())
        {
            if (dynamic_cast<Engine::Component::ULineBatchComponent*>(Component) != nullptr)
            {
                Actor->SetPickable(false);
                return;
            }
        }
    }

    AActor* PersistentActor = new AActor();
    PersistentActor->SetPickable(false);
    PersistentActor->SetRootComponent(new Engine::Component::ULineBatchComponent());
    PersistentScene->AddActor(PersistentActor);
}
