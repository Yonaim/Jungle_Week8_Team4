#include "Editor/UI/EditorPlaceActorsPanel.h"

#include "Editor/EditorEngine.h"
#include "Engine/Platform/Paths.h"
#include "GameFramework/AmbientLightActor.h"
#include "GameFramework/DecalActor.h"
#include "GameFramework/DirectionalLightActor.h"
#include "GameFramework/FakeLightActor.h"
#include "GameFramework/FireballActor.h"
#include "GameFramework/HeightFogActor.h"
#include "GameFramework/PointLightActor.h"
#include "GameFramework/SpotLightActor.h"
#include "GameFramework/StaticMeshActor.h"
#include "GameFramework/World.h"
#include "ImGui/imgui.h"

#include <iterator>

namespace
{
struct FSpawnEntry
{
    const char* Label;
    void (*Spawn)(UWorld* World, const FVector& SpawnPoint);
};

template <typename TActor, typename... TArgs>
void SpawnActor(UWorld* World, const FVector& SpawnPoint, bool bInsertToOctree, TArgs&&... Args)
{
    TActor* Actor = World->SpawnActor<TActor>();
    Actor->InitDefaultComponents(std::forward<TArgs>(Args)...);
    Actor->SetActorLocation(SpawnPoint);
    if (bInsertToOctree)
    {
        World->InsertActorToOctree(Actor);
    }
}

#define SPAWN_MESH(Label, Path) { Label, [](UWorld* W, const FVector& P) { SpawnActor<AStaticMeshActor>(W, P, true, Path); } }
#define SPAWN_ACTOR(Label, Type, bOctree) { Label, [](UWorld* W, const FVector& P) { SpawnActor<Type>(W, P, bOctree); } }

constexpr FSpawnEntry SpawnTable[] = {
    SPAWN_MESH("Cube", FPaths::ContentRelativePath("Models/_Basic/Cube.OBJ")),
    SPAWN_MESH("Sphere", FPaths::ContentRelativePath("Models/_Basic/Sphere.OBJ")),
    SPAWN_ACTOR("Decal", ADecalActor, true),
    SPAWN_ACTOR("Height Fog", AHeightFogActor, false),
    SPAWN_ACTOR("Fake Light", AFakeLightActor, false),
    SPAWN_ACTOR("Fireball", AFireballActor, false),
    SPAWN_ACTOR("Ambient Light", AAmbientLightActor, false),
    SPAWN_ACTOR("Directional Light", ADirectionalLightActor, false),
    SPAWN_ACTOR("Point Light", APointLightActor, false),
    SPAWN_ACTOR("Spot Light", ASpotLightActor, false),
};

#undef SPAWN_MESH
#undef SPAWN_ACTOR

constexpr int32 SpawnTableSize = static_cast<int32>(std::size(SpawnTable));

const char* GetSpawnLabel(void*, int Index)
{
    return SpawnTable[Index].Label;
}
}

void FEditorPlaceActorsPanel::Initialize(UEditorEngine* InEditorEngine)
{
    FEditorPanel::Initialize(InEditorEngine);
    SelectedPrimitiveType = 0;
}

void FEditorPlaceActorsPanel::Render(float DeltaTime)
{
    (void)DeltaTime;
    if (!EditorEngine)
    {
        return;
    }

    ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(320.0f, 220.0f), ImGuiCond_Once);
    ImGui::Begin("Place Actors");

    ImGui::Combo("Primitive", &SelectedPrimitiveType, GetSpawnLabel, nullptr, SpawnTableSize);
    ImGui::InputInt("Number of Spawn", &NumberOfSpawnedActors, 1, 10);
    ImGui::DragFloat3("Spawn Point", &CurSpawnPoint.X, 0.1f);

    if (ImGui::Button("Spawn", ImVec2(-1.0f, 0.0f)))
    {
        UWorld* World = EditorEngine->GetWorld();
        if (World && SelectedPrimitiveType >= 0 && SelectedPrimitiveType < SpawnTableSize)
        {
            for (int32 SpawnIndex = 0; SpawnIndex < NumberOfSpawnedActors; ++SpawnIndex)
            {
                SpawnTable[SelectedPrimitiveType].Spawn(World, CurSpawnPoint);
            }
        }
        NumberOfSpawnedActors = 1;
    }

    ImGui::End();
}
