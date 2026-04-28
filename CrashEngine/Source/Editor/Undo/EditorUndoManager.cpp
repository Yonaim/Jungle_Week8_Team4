#include "Editor/Undo/EditorUndoManager.h"

#include "Editor/Selection/SelectionManager.h"
#include "GameFramework/AActor.h"
#include "GameFramework/World.h"
#include "Object/Object.h"

namespace
{
bool IsSameTransform(const FEditorActorTransformSnapshot& A, const FEditorActorTransformSnapshot& B)
{
    constexpr float ToleranceSq = 0.0001f;
    return A.ActorUUID == B.ActorUUID &&
           FVector::DistSquared(A.Location, B.Location) <= ToleranceSq &&
           FVector::DistSquared(A.Scale, B.Scale) <= ToleranceSq &&
           A.Rotation == B.Rotation;
}
} // namespace

FEditorUndoManager::~FEditorUndoManager()
{
    Shutdown();
}

void FEditorUndoManager::Init(FSelectionManager* InSelectionManager)
{
    SelectionManager = InSelectionManager;
    if (!SnapshotOuter)
    {
        SnapshotOuter = UObjectManager::Get().CreateObject<UObject>();
    }
}

void FEditorUndoManager::Shutdown()
{
    Clear();

    if (SnapshotOuter)
    {
        UObjectManager::Get().DestroyObject(SnapshotOuter);
        SnapshotOuter = nullptr;
    }

    SelectionManager = nullptr;
}

void FEditorUndoManager::Clear()
{
    DestroyActions(UndoStack);
    DestroyActions(RedoStack);
    CancelTransform();
}

void FEditorUndoManager::RecordCreate(UWorld* World, const TArray<AActor*>& CreatedActors)
{
    if (!World || CreatedActors.empty())
    {
        return;
    }

    std::unique_ptr<FEditorUndoAction> Action = std::make_unique<FEditorUndoAction>();
    Action->Type = EEditorUndoActionType::Create;
    Action->World = World;

    for (AActor* Actor : CreatedActors)
    {
        if (!Actor || Actor->GetWorld() != World)
        {
            continue;
        }

        if (AActor* Snapshot = MakeSnapshot(Actor))
        {
            Action->ActorUUIDs.push_back(Actor->GetUUID());
            Action->Snapshots.push_back(Snapshot);
        }
    }

    PushAction(std::move(Action));
}

void FEditorUndoManager::RecordDelete(UWorld* World, const TArray<AActor*>& ActorsToDelete)
{
    if (!World || ActorsToDelete.empty())
    {
        return;
    }

    std::unique_ptr<FEditorUndoAction> Action = std::make_unique<FEditorUndoAction>();
    Action->Type = EEditorUndoActionType::Delete;
    Action->World = World;

    for (AActor* Actor : ActorsToDelete)
    {
        if (!Actor || Actor->GetWorld() != World)
        {
            continue;
        }

        if (AActor* Snapshot = MakeSnapshot(Actor))
        {
            Action->ActorUUIDs.push_back(Actor->GetUUID());
            Action->Snapshots.push_back(Snapshot);
        }
    }

    PushAction(std::move(Action));
}

void FEditorUndoManager::BeginTransform(UWorld* World, const TArray<AActor*>& Actors)
{
    if (!World || Actors.empty())
    {
        return;
    }

    PendingTransformWorld = World;
    PendingTransformBefore.clear();
    PendingTransformBefore.reserve(Actors.size());

    for (AActor* Actor : Actors)
    {
        if (Actor && Actor->GetWorld() == World)
        {
            PendingTransformBefore.push_back(CaptureTransform(Actor));
        }
    }
}

void FEditorUndoManager::EndTransform()
{
    if (!PendingTransformWorld || PendingTransformBefore.empty())
    {
        CancelTransform();
        return;
    }

    std::unique_ptr<FEditorUndoAction> Action = std::make_unique<FEditorUndoAction>();
    Action->Type = EEditorUndoActionType::Transform;
    Action->World = PendingTransformWorld;

    for (const FEditorActorTransformSnapshot& Before : PendingTransformBefore)
    {
        AActor* Actor = FindLiveActor(PendingTransformWorld, Before.ActorUUID);
        if (!Actor)
        {
            continue;
        }

        FEditorActorTransformSnapshot After = CaptureTransform(Actor);
        if (IsSameTransform(Before, After))
        {
            continue;
        }

        Action->ActorUUIDs.push_back(Before.ActorUUID);
        Action->BeforeTransforms.push_back(Before);
        Action->AfterTransforms.push_back(After);
    }

    CancelTransform();
    PushAction(std::move(Action));
}

void FEditorUndoManager::CancelTransform()
{
    PendingTransformWorld = nullptr;
    PendingTransformBefore.clear();
}

void FEditorUndoManager::Undo()
{
    if (UndoStack.empty())
    {
        return;
    }

    std::unique_ptr<FEditorUndoAction> Action = std::move(UndoStack.back());
    UndoStack.pop_back();

    if (!Action || !Action->World)
    {
        return;
    }

    switch (Action->Type)
    {
    case EEditorUndoActionType::Create:
        if (SelectionManager)
        {
            SelectionManager->ClearSelection();
        }
        for (uint32 ActorUUID : Action->ActorUUIDs)
        {
            DeleteActorByUUID(Action->World, ActorUUID);
        }
        break;

    case EEditorUndoActionType::Delete:
        for (int32 Index = 0; Index < static_cast<int32>(Action->Snapshots.size()); ++Index)
        {
            const uint32 ActorUUID = Index < static_cast<int32>(Action->ActorUUIDs.size()) ? Action->ActorUUIDs[Index] : 0;
            RestoreSnapshot(Action->World, Action->Snapshots[Index], ActorUUID);
        }
        SelectActors(Action->World, Action->ActorUUIDs);
        break;

    case EEditorUndoActionType::Transform:
        for (const FEditorActorTransformSnapshot& Snapshot : Action->BeforeTransforms)
        {
            ApplyTransform(Action->World, Snapshot);
        }
        SelectActors(Action->World, Action->ActorUUIDs);
        break;
    }

    RedoStack.push_back(std::move(Action));
}

void FEditorUndoManager::Redo()
{
    if (RedoStack.empty())
    {
        return;
    }

    std::unique_ptr<FEditorUndoAction> Action = std::move(RedoStack.back());
    RedoStack.pop_back();

    if (!Action || !Action->World)
    {
        return;
    }

    switch (Action->Type)
    {
    case EEditorUndoActionType::Create:
        for (int32 Index = 0; Index < static_cast<int32>(Action->Snapshots.size()); ++Index)
        {
            const uint32 ActorUUID = Index < static_cast<int32>(Action->ActorUUIDs.size()) ? Action->ActorUUIDs[Index] : 0;
            RestoreSnapshot(Action->World, Action->Snapshots[Index], ActorUUID);
        }
        SelectActors(Action->World, Action->ActorUUIDs);
        break;

    case EEditorUndoActionType::Delete:
        if (SelectionManager)
        {
            SelectionManager->ClearSelection();
        }
        for (uint32 ActorUUID : Action->ActorUUIDs)
        {
            DeleteActorByUUID(Action->World, ActorUUID);
        }
        break;

    case EEditorUndoActionType::Transform:
        for (const FEditorActorTransformSnapshot& Snapshot : Action->AfterTransforms)
        {
            ApplyTransform(Action->World, Snapshot);
        }
        SelectActors(Action->World, Action->ActorUUIDs);
        break;
    }

    UndoStack.push_back(std::move(Action));
}

FEditorActorTransformSnapshot FEditorUndoManager::CaptureTransform(AActor* Actor) const
{
    FEditorActorTransformSnapshot Snapshot{};
    if (!Actor)
    {
        return Snapshot;
    }

    Snapshot.ActorUUID = Actor->GetUUID();
    Snapshot.Location = Actor->GetActorLocation();
    Snapshot.Rotation = Actor->GetActorRotation();
    Snapshot.Scale = Actor->GetActorScale();
    return Snapshot;
}

void FEditorUndoManager::ApplyTransform(UWorld* World, const FEditorActorTransformSnapshot& Snapshot)
{
    AActor* Actor = FindLiveActor(World, Snapshot.ActorUUID);
    if (!Actor)
    {
        return;
    }

    Actor->SetActorLocation(Snapshot.Location);
    Actor->SetActorRotation(Snapshot.Rotation);
    Actor->SetActorScale(Snapshot.Scale);

    if (World)
    {
        World->UpdateActorInOctree(Actor);
        World->MarkEditorPickingAndScenePrimitiveBVHsDirty();
    }
}

AActor* FEditorUndoManager::FindLiveActor(UWorld* World, uint32 ActorUUID) const
{
    if (!World || ActorUUID == 0)
    {
        return nullptr;
    }

    AActor* Actor = Cast<AActor>(UObjectManager::Get().FindByUUID(ActorUUID));
    if (Actor && Actor->GetWorld() == World)
    {
        return Actor;
    }

    for (AActor* Candidate : World->GetActors())
    {
        if (Candidate && Candidate->GetUUID() == ActorUUID)
        {
            return Candidate;
        }
    }

    return nullptr;
}

AActor* FEditorUndoManager::MakeSnapshot(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }

    if (!SnapshotOuter)
    {
        SnapshotOuter = UObjectManager::Get().CreateObject<UObject>();
    }

    return Cast<AActor>(Actor->Duplicate(SnapshotOuter));
}

AActor* FEditorUndoManager::RestoreSnapshot(UWorld* World, AActor* Snapshot, uint32 ActorUUID)
{
    if (!World || !Snapshot)
    {
        return nullptr;
    }

    AActor* Restored = Cast<AActor>(Snapshot->Duplicate(World));
    if (!Restored)
    {
        return nullptr;
    }

    if (ActorUUID != 0)
    {
        Restored->SetUUID(ActorUUID);
    }

    World->MarkEditorPickingAndScenePrimitiveBVHsDirty();
    return Restored;
}

void FEditorUndoManager::DeleteActorByUUID(UWorld* World, uint32 ActorUUID)
{
    AActor* Actor = FindLiveActor(World, ActorUUID);
    if (!Actor)
    {
        return;
    }

    if (SelectionManager)
    {
        SelectionManager->Deselect(Actor);
    }

    World->DestroyActor(Actor);
}

void FEditorUndoManager::SelectActors(UWorld* World, const TArray<uint32>& ActorUUIDs)
{
    if (!SelectionManager)
    {
        return;
    }

    SelectionManager->ClearSelection();
    for (uint32 ActorUUID : ActorUUIDs)
    {
        if (AActor* Actor = FindLiveActor(World, ActorUUID))
        {
            SelectionManager->ToggleSelect(Actor);
        }
    }
}

void FEditorUndoManager::PushAction(std::unique_ptr<FEditorUndoAction> Action)
{
    if (!Action)
    {
        return;
    }

    const bool bEmptyAction =
        Action->ActorUUIDs.empty() &&
        Action->BeforeTransforms.empty() &&
        Action->AfterTransforms.empty();
    if (bEmptyAction)
    {
        DestroyAction(Action.get());
        return;
    }

    DestroyActions(RedoStack);
    UndoStack.push_back(std::move(Action));
}

void FEditorUndoManager::DestroyAction(FEditorUndoAction* Action)
{
    if (!Action)
    {
        return;
    }

    for (AActor* Snapshot : Action->Snapshots)
    {
        if (Snapshot)
        {
            UObjectManager::Get().DestroyObject(Snapshot);
        }
    }
    Action->Snapshots.clear();
}

void FEditorUndoManager::DestroyActions(TArray<std::unique_ptr<FEditorUndoAction>>& Actions)
{
    for (std::unique_ptr<FEditorUndoAction>& Action : Actions)
    {
        DestroyAction(Action.get());
    }
    Actions.clear();
}
