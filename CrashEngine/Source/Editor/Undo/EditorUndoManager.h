#pragma once

#include <memory>

#include "Core/CoreTypes.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

class AActor;
class FSelectionManager;
class UObject;
class UWorld;

struct FEditorActorTransformSnapshot
{
    uint32 ActorUUID = 0;
    FVector Location = FVector(0.0f, 0.0f, 0.0f);
    FRotator Rotation;
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);
};

enum class EEditorUndoActionType : uint8
{
    Create,
    Delete,
    Transform,
};

struct FEditorUndoAction
{
    EEditorUndoActionType Type = EEditorUndoActionType::Create;
    UWorld* World = nullptr;

    TArray<uint32> ActorUUIDs;
    TArray<AActor*> Snapshots;

    TArray<FEditorActorTransformSnapshot> BeforeTransforms;
    TArray<FEditorActorTransformSnapshot> AfterTransforms;
};

class FEditorUndoManager
{
public:
    FEditorUndoManager() = default;
    ~FEditorUndoManager();

    void Init(FSelectionManager* InSelectionManager);
    void Shutdown();
    void Clear();

    void RecordCreate(UWorld* World, const TArray<AActor*>& CreatedActors);
    void RecordDelete(UWorld* World, const TArray<AActor*>& ActorsToDelete);

    void BeginTransform(UWorld* World, const TArray<AActor*>& Actors);
    void EndTransform();
    void CancelTransform();

    void Undo();
    void Redo();

private:
    FEditorActorTransformSnapshot CaptureTransform(AActor* Actor) const;
    void ApplyTransform(UWorld* World, const FEditorActorTransformSnapshot& Snapshot);

    AActor* FindLiveActor(UWorld* World, uint32 ActorUUID) const;
    AActor* MakeSnapshot(AActor* Actor);
    AActor* RestoreSnapshot(UWorld* World, AActor* Snapshot, uint32 ActorUUID);
    void DeleteActorByUUID(UWorld* World, uint32 ActorUUID);
    void SelectActors(UWorld* World, const TArray<uint32>& ActorUUIDs);

    void PushAction(std::unique_ptr<FEditorUndoAction> Action);
    void DestroyAction(FEditorUndoAction* Action);
    void DestroyActions(TArray<std::unique_ptr<FEditorUndoAction>>& Actions);

private:
    FSelectionManager* SelectionManager = nullptr;
    UObject* SnapshotOuter = nullptr;

    TArray<std::unique_ptr<FEditorUndoAction>> UndoStack;
    TArray<std::unique_ptr<FEditorUndoAction>> RedoStack;

    UWorld* PendingTransformWorld = nullptr;
    TArray<FEditorActorTransformSnapshot> PendingTransformBefore;
};
